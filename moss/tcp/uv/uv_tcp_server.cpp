#include "uv_tcp_server.h"

#include <atomic>
#include <deque>
#include <sstream>
#include <thread>
#include <unordered_map>
#include "uv_types.h"
#include "uv_worker.h"
#include "uv_connection.h"
#include "../tcp_event_handler.h"
#include "../tcp_server.h"
#include "utils/logger.h"


using std::thread;
namespace moss {
	namespace {
		using IpcWorker = struct {
			union {
				uv_tcp_t stream_;
				uv_pipe_t pipe_;
			};
			uv_write_t req_;
		};
		void IpcCloseCallback(uv_handle_t* handle) {
			IpcWorker* worker = container_of(handle, IpcWorker, stream_);
			free(worker);
		}

		void IpcWriteCallback(uv_write_t* req, int status) {
			IpcWorker* worker = container_of(req, IpcWorker, req_);
			uv_close((uv_handle_t*)&worker->stream_, IpcCloseCallback);
		}

		void IpcAcceptCallback(uv_stream_t* ipc, int status) {
			if (status < 0) {
				return;
			}
			TcpServerImpl* server = (TcpServerImpl*)(uv_handle_get_data((uv_handle_t*)ipc));
			if (!server) {
				return;
			}
			server->AcceptWorker();
		}			
	}

	TcpServerImpl::TcpServerImpl(shared_ptr<TcpServer> server)
		: server_(server),
		loop_(std::make_shared<uv_loop_t>()),
		listener_(std::make_shared<uv_tcp_t>()),
		ipc_(std::make_shared<uv_pipe_t>()),
		mutex_(std::make_shared<mutex>()),
		ready_workers_(0) {
		uv_loop_init(loop_.get());
		uv_loop_set_data(loop_.get(), this);
	}

	TcpServerImpl::~TcpServerImpl() {
		uv_close((uv_handle_t*)listener_.get(), nullptr);
		uv_loop_close(loop_.get());
	}

	shared_ptr<uv_loop_t> TcpServerImpl::GetLoop() const {
		return loop_;
	}

	int TcpServerImpl::SetupWorkers(int workers) {
		for (int i = 0; i < workers; i++) {
			auto worker = std::make_shared<UvWorker>(i, shared_from_this());
			worker->Setup();
			workers_.push_back(worker);
		}
		return 0;
	}

	void TcpServerImpl::StartWorkers() {
		for (auto it : workers_) {
			it->Start();
		}
	}

	void TcpServerImpl::AcceptWorker() {
		static const char* ping = "PING";
		IpcWorker* worker = new IpcWorker();
		uv_buf_t buf = uv_buf_init((char*)ping, 4);
		uv_pipe_init(loop_.get(), &(worker->pipe_), 1);
		uv_accept((uv_stream_t*)ipc_.get(), (uv_stream_t*)&(worker->stream_));
		uv_write2(&worker->req_, (uv_stream_t*)&(worker->stream_), &buf, 1, (uv_stream_t*)listener_.get(), &IpcWriteCallback);
		if (++ready_workers_ == (int)workers_.size()) {
			uv_close((uv_handle_t*)ipc_.get(), nullptr);
		}
	}

	shared_ptr<UvWorker> TcpServerImpl::GetWorker(int worker_id) {
		if (worker_id >= (int)workers_.size())
			return nullptr;
		auto worker = workers_[worker_id];
		return worker;
	}

	int TcpServerImpl::Start(const string& ip, int port, int num_of_workers/* = 16*/) {
		int retval = -1;
		do {
			struct sockaddr_in listen_addr;
			retval = uv_ip4_addr(ip.c_str(), port, &listen_addr);
			if (retval != 0) {
				logger::Fatal() << "uv_ip4_addr failed: " << uv_strerror(retval);
				break;
			}
			retval = uv_tcp_init(loop_.get(), listener_.get());
			if (retval != 0) {
				logger::Fatal() << "uv_tcp_init failed: " << uv_strerror(retval);
				break;
			}
			retval = uv_tcp_bind(listener_.get(), (const struct sockaddr*)&listen_addr, 0);
			if (retval != 0) {
				logger::Fatal() << "uv_tcp_bind failed: " << uv_strerror(retval);
				break;
			}
			SetupWorkers(num_of_workers);
			retval = uv_pipe_init(loop_.get(), ipc_.get(), 0);
			if (retval != 0) {
				logger::Fatal() << "uv_pipe_init failed: " << uv_strerror(retval);
				break;
			}
			uv_handle_set_data((uv_handle_t*)ipc_.get(), this);
			uv_pipe_pending_instances(ipc_.get(), (int)workers_.size());
			retval = uv_pipe_bind(ipc_.get(), MIRABILIS_CHANNEL_TCP_LISTENER);
			if (retval != 0) {
				logger::Fatal() << "uv_pipe_bind failed: " << uv_strerror(retval);
				break;
			}
			retval = uv_listen((uv_stream_t*)ipc_.get(), SOMAXCONN, &IpcAcceptCallback);
			if (retval != 0) {
				logger::Fatal() << "uv_listen failed: " << uv_strerror(retval);
				break;
			}
			StartWorkers();
			retval = uv_run(loop_.get(), UV_RUN_DEFAULT);
			if (retval != 0) {
				logger::Fatal() << "uv_run failed: " << uv_strerror(retval);
				break;
			}
			uv_close((uv_handle_t*)listener_.get(), nullptr);
			return 0;
		} while (0);
		logger::Info() << "listening " << ip << ":" << port;
		return retval;
	}

	int TcpServerImpl::Stop() {
		for (auto worker : workers_) {
			worker->Stop();
		}
		uv_stop(loop_.get());
		return 0;
	}

	shared_ptr<TcpServer> TcpServerImpl::GetServer() const {
		auto server = server_.lock();
		return server;
	}

	shared_ptr<TcpEventHandler> TcpServerImpl::GetIoEventHandler() const {
		auto server = GetServer();
		auto tcp_event_handler = server->GetIoEventHandler();
		return tcp_event_handler;
	}

	shared_ptr<UvWorker> TcpServerImpl::GetWorker() const {
		static int worker_id = 0;
		if (worker_id >= (int)workers_.size()) {
			worker_id = 0;
		}
		return workers_[worker_id++];
	}
} // namespace moss


