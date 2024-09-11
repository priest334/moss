#include "tcp_server.h"

#include "tcp_event_handler.h"
#include "uv/uv_tcp_server.h"


namespace moss {
	TcpServer::TcpServer(shared_ptr<TcpEventHandler> tcp_event_handler)
		: tcp_event_handler_(tcp_event_handler),
		port_(0) {
	}

	TcpServer::~TcpServer() {
	}

	shared_ptr<TcpEventHandler> TcpServer::GetIoEventHandler() {
		return tcp_event_handler_;
	}

	shared_ptr<TcpServerImpl> TcpServer::GetImpl() {
		return impl_;
	}

	string TcpServer::ListenIp() const {
		return ip_;
	}

	int TcpServer::ListenPort() const {
		return port_;
	}

	int TcpServer::Start(const string& ip, int port) {
		ip_ = ip.c_str();
		port_ = port;
		impl_ = std::make_shared<TcpServerImpl>(shared_from_this());
		return impl_->Start(ip, port, 1);
	}

	int TcpServer::Stop() {
		if (!impl_) {
			return -1;
		}
		return impl_->Stop();
	}
} // namespace moss

