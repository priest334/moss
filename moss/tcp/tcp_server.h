#pragma once

#include <memory>
#include <string>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
namespace moss {
	class TcpEventHandler;
	class TcpServerImpl;
	class TcpServer
		: public std::enable_shared_from_this<TcpServer> {
		friend class TcpServerImpl;
	public:
		MOSS_EXPORT TcpServer(shared_ptr<TcpEventHandler> tcp_event_handler);
		MOSS_EXPORT ~TcpServer();
		shared_ptr<TcpEventHandler> GetIoEventHandler();
		shared_ptr<TcpServerImpl> GetImpl();
		MOSS_EXPORT string ListenIp() const;
		MOSS_EXPORT int ListenPort() const;
		MOSS_EXPORT int Start(const string& ip, int port);
		MOSS_EXPORT int Stop();
	private:
		shared_ptr<TcpEventHandler> tcp_event_handler_;
		shared_ptr<TcpServerImpl> impl_;
		string ip_;
		int port_;
	};
} // namespace moss

