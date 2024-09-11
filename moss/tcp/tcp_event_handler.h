#pragma once

#include <memory>
#include <string>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
namespace moss {
	class Connection;
	class MOSS_EXPORT TcpEventHandler {
	public:
		virtual ~TcpEventHandler();
		virtual int OnCreate(shared_ptr<Connection> connection) = 0;
		virtual int OnRead(shared_ptr<Connection> connection, shared_ptr<string> rdbuf, int size) = 0;
		virtual int OnWrite(shared_ptr<Connection> connection, shared_ptr<string> wrbuf, int status) = 0;
		virtual int OnClose(shared_ptr<Connection> connection) = 0;
		virtual int OnError(int64_t id, const string& message) = 0;
	};
} // namespace moss


