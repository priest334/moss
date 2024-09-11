#pragma once

#include <atomic>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include "../../tcp/tcp_event_handler.h"


using std::mutex;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::weak_ptr;
namespace moss {
	namespace http {
		class Application;
		class Session;
		class Request;
		class Response;
	}
	class TaskRunner;
	class TimerLoop;
	class Connection;
	class TcpServer;
	class HttpServer;
	class HttpServerImpl
		: public TcpEventHandler,
		public std::enable_shared_from_this<HttpServerImpl> {
		using Sessions = unordered_map<int64_t, shared_ptr<http::Session>>;
		using Applications = unordered_map<string, shared_ptr<http::Application>>;
	public:
		HttpServerImpl(weak_ptr<HttpServer> context);
		int Start(const string& ip, int port, int workers);
		int Stop();
		int OnCreate(shared_ptr<Connection> connection) override;
		int OnRead(shared_ptr<Connection> connection, shared_ptr<string> rdbuf, int size) override;
		int OnWrite(shared_ptr<Connection> connection, shared_ptr<string> wrbuf, int status) override;
		int OnClose(shared_ptr<Connection> connection) override;
		int OnError(int64_t id, const string& message) override;
		int CheckRequestTimeout();
		int Process(shared_ptr<http::Request> request, shared_ptr<http::Response> response);
		int CloseSession(shared_ptr<http::Session> session);
	protected:
		weak_ptr<HttpServer> context_;
		std::atomic_int64_t session_id_seq_;
		shared_ptr<TcpServer> server_;
		shared_ptr<mutex> mutex_;
		Sessions sessions_;
		shared_ptr<TaskRunner> task_runner_;
		shared_ptr<TimerLoop> timer_loop_;
		time_t read_timeout_;
		time_t write_timeout_;
	};
} // namespace moss

