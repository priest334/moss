#pragma once

#include <atomic>
#include <ctime>
#include <memory>
#include <string>


using std::shared_ptr;
using std::string;
using std::weak_ptr;
namespace moss {
	class Connection;
	class HttpServerImpl;
	namespace http {
		class Request;
		class RequestParser;
		class Session
			: public std::enable_shared_from_this<Session> {
			friend class HttpServer;
		public:
			Session(int64_t id, shared_ptr<Connection> connection, shared_ptr<HttpServerImpl> server);
			~Session();
			int64_t Id() const;
			string Ip() const;
			shared_ptr<Connection> GetConnection() const;
			void Close();
			bool IsClosing() const;
			void ReadComplete();
			bool IsReadCompleted() const;
			void LastRead(int64_t last_read);
			int64_t LastRead() const;
			void LastWrite(int64_t last_write);
			int64_t LastWrite() const;
			void ResetTimeout();
			int CreateParser();
			void ResetParser();
			shared_ptr<Request> Append(shared_ptr<string> rdbuf, size_t size);
			int Write(shared_ptr<string> wrbuf);
		private:
			int64_t id_;
			weak_ptr<Connection> connection_;
			weak_ptr<HttpServerImpl> server_;
			string ip_;
			shared_ptr<RequestParser> request_parser_;
			std::atomic_bool closing_;
			std::atomic_bool read_completed_;
			std::atomic_int64_t last_read_;
			std::atomic_int64_t last_write_;
		};
	}
} // namespace moss


