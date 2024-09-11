#include "uv_connection.h"

#include "utils/logger.h"
#include "uv_worker.h"
#include "uv_tcp_server.h"
#include "../tcp_event_handler.h"


namespace moss {
	namespace {
		shared_ptr<UvConnection> SharedFromHandle(void* handle) {
			UvConnection* pthis = static_cast<UvConnection*>(uv_handle_get_data(static_cast<uv_handle_t*>(handle)));
			if (!pthis)
				return nullptr;
			return pthis->SharedFromPodPointer();
		}
	}
	void AllocCallback(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
		UvConnection* connection = static_cast<UvConnection*>(uv_handle_get_data(handle));
		if (!connection)
			return;
		auto rdbuf = connection->ReadBuffer();
		if (rdbuf->size() < suggested_size) {
			rdbuf->resize(suggested_size);
		}
		buf->base = (char*)rdbuf->c_str();
		buf->len = (unsigned long)suggested_size;
	}

	void ReadCallback(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
		auto connection = SharedFromHandle(handle);
		if (!connection)
			return;
		auto tcp_event_handler = connection->GetIoEventHandler();
		if (nread > 0) {
			if (tcp_event_handler) {
				tcp_event_handler->OnRead(connection, connection->ReadBuffer(), (int)nread);
			}
		} else {
#ifdef _WIN32
			if (WSAEWOULDBLOCK != WSAGetLastError()) {
#else
			if (errno != EAGAIN) {
#endif
				if (tcp_event_handler) {
					if (nread != UV_EOF) {
						tcp_event_handler->OnError(connection->Id(), uv_err_name((int)nread));
					}
					tcp_event_handler->OnClose(connection);
				}
				//connection->Close();
				connection->Cleanup();
			}
		}
	}

	void WriteCallback(uv_write_t* req, int status) {
		auto connection = SharedFromHandle(req);
		free(req);
		if (!connection)
			return;
		auto tcp_event_handler = connection->GetIoEventHandler();
		if (tcp_event_handler) {
			tcp_event_handler->OnWrite(connection, connection->WriteBuffer(), status);
		}
		connection->WriteFinished();
	}

	void CloseCallback(uv_handle_t* handle) {
		auto connection = SharedFromHandle(handle);
		if (!connection)
			return;
		auto tcp_event_handler = connection->GetIoEventHandler();
		if (tcp_event_handler) {
			tcp_event_handler->OnClose(connection);
		}
		connection->Cleanup();
	}

	UvConnection::UvConnection(int64_t id, shared_ptr<uv_tcp_t> handle, shared_ptr<UvWorker> worker)
		: Connection(id),
		worker_(worker),
		handle_(handle),
		rdbuf_(std::make_shared<string>()),
		mutex_(std::make_shared<mutex>()),
		wq_(std::make_shared<WriteQueue>()),
		wqs_(std::make_shared<WriteQueue>()) {
		uv_handle_set_data((uv_handle_t*)handle_.get(), this);
		ip_ = std::make_shared<string>(GetIp());
		//moss::logger::Debug() << "UvConnection: " << id;
	}

	UvConnection::~UvConnection() {
		//moss::logger::Debug() << "~UvConnection: " << Id();
	}

	shared_ptr<UvWorker> UvConnection::GetWorker() const {
		return worker_.lock();
	}

	shared_ptr<TcpEventHandler> UvConnection::GetIoEventHandler() const {
		auto worker = GetWorker();
		if (!worker)
			return nullptr;
		return worker->GetIoEventHandler();
	}

	shared_ptr<UvConnection> UvConnection::SharedFromPodPointer() const {
		auto worker = GetWorker();
		if (!worker)
			return nullptr;
		return worker->GetConnection(Id());
	}

	shared_ptr<TcpServerImpl> UvConnection::GetUvTcpServer() const {
		auto worker = GetWorker();
		if (!worker)
			return nullptr;
		return worker->GetServer();
	}

	uv_tcp_t* UvConnection::Handle() {
		return handle_.get();
	}

	shared_ptr<string> UvConnection::ReadBuffer() {
		return rdbuf_;
	}

	shared_ptr<string> UvConnection::WriteBuffer() {
		if (wqs_->empty())
			return nullptr;
		return wqs_->front();
	}

	void UvConnection::WriteFinished() {
		if (wqs_->empty())
			return;
		wqs_->pop_front();
	}

	void UvConnection::Cleanup() {
		auto worker = GetWorker();
		if (!worker)
			return;
		worker->CloseConnection(Id());
	}

	void UvConnection::Start() {
		uv_read_start((uv_stream_t*)handle_.get(), &AllocCallback, &ReadCallback);
	}

	int UvConnection::Write(shared_ptr<string> wrbuf) {
		auto worker = worker_.lock();
		if (!worker)
			return -1;
		std::lock_guard<mutex> lock(*mutex_);
		wq_->push_back(wrbuf);
		worker->Write(Id());
		return 0;
	}

	int UvConnection::Close() {
		auto worker = worker_.lock();
		if (!worker)
			return -1;
		std::lock_guard<mutex> lock(*mutex_);
		wq_->push_back(nullptr);
		worker->Write(Id());
		return 0;
	}

	string UvConnection::Ip() const {
		if (!ip_)
			return string();
		return *ip_;
	}

	int UvConnection::Write() {
		int write_count = 0;
		{
			std::lock_guard<mutex> lock(*mutex_);
			if (!wq_->empty()) {
				wqs_->swap(*wq_);
				//wqs_->insert(wqs_->end(), wq_->begin(), wq_->end());
				//wq_->clear();
			}
		}
		for (auto wrbuf: *wqs_) {
			if (!wrbuf) {
				//if(uv_is_active((uv_handle_t*)handle_.get())) {
				//	Write(nullptr);
				//	continue;
				//}
				uv_close((uv_handle_t*)handle_.get(), &CloseCallback);
				//uv_tcp_close_reset(handle_.get(), &CloseCallback);
				wqs_->clear();
				write_count++;
				break;
			}
			uv_buf_t buffer;
			buffer.base = (char*)wrbuf->c_str();
			buffer.len = (unsigned long)wrbuf->size();
			uv_write_t* req = (uv_write_t*)calloc(1, sizeof(uv_write_t));
			uv_handle_set_data((uv_handle_t*)req, this);
			int retval = uv_write(req, (uv_stream_t*)handle_.get(), &buffer, 1, &WriteCallback);
			if (0 != retval) {
				moss::logger::Debug(__FILE__, __LINE__) << "uv_write: wrlen->" << wrbuf->size() << ", retval->" << retval;
			}
			write_count += (0 == retval) ? 1 : 0;
		}		
		return write_count;
	}

	string UvConnection::GetIp() const {
		const int max_ip_length = 64;
		char buffer[max_ip_length] = { 0 };
		struct sockaddr_storage ss;
		int namelen = sizeof(ss);
		uv_tcp_getpeername(handle_.get(), (sockaddr*)&ss, &namelen);
		if (ss.ss_family == AF_INET) {
			struct sockaddr_in* addr = (struct sockaddr_in*)&ss;
			inet_ntop(addr->sin_family, &(addr->sin_addr), buffer, max_ip_length);
		} else {
			struct sockaddr_in6* addr = (struct sockaddr_in6*)&ss;
			inet_ntop(addr->sin6_family, &(addr->sin6_addr), buffer, max_ip_length);
		}
		return string(buffer);
	}
} // namespace moss


