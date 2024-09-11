#include "uv_worker.h"

#include "uv_types.h"
#include "uv_connection.h"
#include "uv_tcp_server.h"
#include "../tcp_event_handler.h"
#include "utils/logger.h"


namespace moss {
	namespace {
		shared_ptr<UvWorker> SharedFromLoop(uv_loop_t* loop) {
			UvWorker* worker = static_cast<UvWorker*>(uv_loop_get_data(loop));
			return worker->SharedFromPodPointer();
		}

		shared_ptr<UvWorker> SharedFromHandle(void* handle) {
			uv_loop_t* loop = uv_handle_get_loop(static_cast<uv_handle_t*>(handle));
			if (!loop)
				return nullptr;
			return SharedFromLoop(loop);
		}

		void UvWorkerThreadProc(shared_ptr<UvWorker> worker) {
			worker->Run();
		}

		void IpcAllocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
			auto worker = SharedFromHandle(handle);
			auto scratch = worker->Scratch(suggested_size);
			buf->base = (char*)scratch->c_str();
			buf->len = (unsigned long)scratch->size();
		}

		void IpcReadCallback(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
			auto worker = SharedFromHandle(handle);
			auto loop = worker->GetLoop();
			auto ipc = worker->Pipe();
			auto listener = worker->Listener();
			int retval = uv_pipe_pending_count(ipc.get());
			if (retval <= 0)
				return;
			uv_handle_type type = uv_pipe_pending_type(ipc.get());
			if (type != UV_TCP)
				return;
			uv_tcp_init(loop.get(), listener.get());
			uv_handle_set_data((uv_handle_t*)listener.get(), worker.get());
			retval = uv_accept(handle, reinterpret_cast<uv_stream_t*>(listener.get()));
			if (0 == retval) {
				worker->Listen();
			}
			uv_close((uv_handle_t*)ipc.get(), nullptr);
		}

		void IpcConnectCallback(uv_connect_t* req, int status) {
			if (status != 0)
				return;
			auto worker = SharedFromHandle(req->handle);
			auto ipc = worker->Pipe();
			uv_read_start((uv_stream_t*)ipc.get(), IpcAllocCallback, IpcReadCallback);
		}

		void AcceptCallback(uv_stream_t* server_handle, int status) {
			if (status < 0) {
				return;
			}
			auto worker = SharedFromHandle(server_handle);
			if (!worker)
				return;
			worker->Accept();
		}

		void AsyncCallback(uv_async_t* handle) {
			auto worker = SharedFromHandle(handle);
			if (!worker)
				return;
			worker->Write();
		}
	}

	UvWorker::UvWorker(worker_id_t id, shared_ptr<TcpServerImpl> server)
		: id_(id),
		server_(server),
		loop_(std::make_shared<uv_loop_t>()),
		async_(std::make_shared<uv_async_t>()),
		mutex_(std::make_shared<mutex>()),
		jobs_(std::make_shared<WriteJobs>()),
		listener_(std::make_shared<uv_tcp_t>()),
		semaphore_(std::make_shared<uv_sem_t>()),
		pipe_(std::make_shared<uv_pipe_t>()),
		scratch_(std::make_shared<string>()),
		connections_mutex_(std::make_shared<mutex>()),
		connection_id_(1) {
		uv_loop_init(loop_.get());
		uv_loop_set_data(loop_.get(), this);
		uv_async_init(loop_.get(), async_.get(), &AsyncCallback);
	}

	int UvWorker::Id() const {
		return id_;
	}

	shared_ptr<TcpServerImpl> UvWorker::GetServer() const {
		return server_.lock();
	}

	shared_ptr<TcpEventHandler> UvWorker::GetIoEventHandler() const {
		auto server = server_.lock();
		return server->GetIoEventHandler();
	}

	shared_ptr<UvWorker> UvWorker::SharedFromPodPointer() const {
		auto server = server_.lock();
		return server->GetWorker(id_);
	}

	shared_ptr<uv_loop_t> UvWorker::GetLoop() const {
		return loop_;
	}

	shared_ptr<uv_tcp_t> UvWorker::Listener() const {
		return listener_;
	}

	shared_ptr<uv_tcp_t> UvWorker::Listener() {
		return listener_;
	}

	shared_ptr<uv_pipe_t> UvWorker::Pipe() const {
		return pipe_;
	}

	shared_ptr<string> UvWorker::Scratch() const {
		return scratch_;
	}

	shared_ptr<string> UvWorker::Scratch(size_t size) {
		if (scratch_->size() < size) {
			scratch_->resize(size);
		}
		return scratch_;
	}

	void UvWorker::Setup() {
		uv_sem_init(semaphore_.get(), 0);
		thread_ = std::make_shared<thread>(&UvWorkerThreadProc, shared_from_this());
	}

	void UvWorker::Start() {
		uv_sem_post(semaphore_.get());
	}

	void UvWorker::Listen() {
		uv_listen((uv_stream_t*)listener_.get(), SOMAXCONN, &AcceptCallback);
	}

	void UvWorker::Stop() {
		uv_loop_close(loop_.get());
		if (thread_) {
			thread_->join();
		}
	}

	void UvWorker::Run() {
		uv_connect_t ipc_connect_req;
		uv_handle_set_data((uv_handle_t*)&ipc_connect_req, this);
		uv_sem_wait(semaphore_.get());
		uv_pipe_init(loop_.get(), pipe_.get(), 1);
		uv_handle_set_data((uv_handle_t*)pipe_.get(), this);
		uv_pipe_connect(&ipc_connect_req, pipe_.get(), MIRABILIS_CHANNEL_TCP_LISTENER, &IpcConnectCallback);
		uv_run(loop_.get(), UV_RUN_DEFAULT);
		uv_loop_close(loop_.get());
	}

	shared_ptr<UvConnection> UvWorker::CreateConnection(shared_ptr<uv_tcp_t> handle) {
		auto connection_id = connection_id_.fetch_add(1);
		auto connection = std::make_shared<UvConnection>(connection_id, handle, shared_from_this());
		std::lock_guard<std::mutex> lock(*connections_mutex_);
		connections_[connection_id] = connection;
		return connection;
	}

	shared_ptr<UvConnection> UvWorker::GetConnection(int64_t id) const {
		std::lock_guard<std::mutex> lock(*connections_mutex_);
		auto it = connections_.find(id);
		if (it != connections_.end()) {
			return std::static_pointer_cast<UvConnection>(it->second);
		}
		return nullptr;
	}

	void UvWorker::CloseConnection(int64_t id) {
		std::lock_guard<std::mutex> lock(*connections_mutex_);
		auto it = connections_.find(id);
		if (it != connections_.end()) {
			connections_.erase(it);
		}
	}

	void UvWorker::Accept() {
		auto handle = std::make_shared<uv_tcp_t>();
		uv_tcp_init(loop_.get(), handle.get());
		uv_tcp_nodelay(handle.get(), 1);
		uv_tcp_keepalive(handle.get(), 1, 60);
		uv_stream_set_blocking((uv_stream_t*)handle.get(), 0);
		int retval = uv_accept((uv_stream_t*)listener_.get(), (uv_stream_t*)handle.get());
		if (0 != retval) {
			return;
		}
		auto connection = CreateConnection(handle);
		auto tcp_event_handler = GetIoEventHandler();
		if (tcp_event_handler) {
			tcp_event_handler->OnCreate(connection);
		}
		connection->Start();
	}

	void UvWorker::Write(int64_t connection_id) {
		std::lock_guard<mutex> lock(*mutex_);
		auto it = jobs_->find(connection_id);
		if (it != jobs_->end()) {
			++it->second;
		} else {
			jobs_->insert(std::make_pair(connection_id, 1));
		}
		uv_async_send(async_.get());
	}

	void UvWorker::Write() {
		auto jobs = std::make_shared<WriteJobs>();
		{
			std::lock_guard<mutex> lock(*mutex_);
			jobs->swap(*jobs_);
		}
		for (auto& job : *jobs) {
			auto connection = GetConnection(job.first);
			if (!connection) {
				continue;
			}
			connection->Write();
		}
	}
} // namespace moss


