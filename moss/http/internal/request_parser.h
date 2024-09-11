#pragma once

#include <memory>
#include <string>


using std::shared_ptr;
using std::string;
using std::weak_ptr;
namespace moss {
	namespace http {
		class Session;
		class Request;
		class RequestParserContext;
		class RequestParser
			: public std::enable_shared_from_this<RequestParser> {
			friend class RequestParserContext;
		public:
			RequestParser(shared_ptr<Session> session);
			void Initalize();
			void Reset();
			size_t Parse(const char* data, size_t len);
			void PrepareRequest();
			void CompleteRequest();
			bool IsRequestCompleted() const;
			shared_ptr<Request> GetRequest();
		private:
			weak_ptr<Session> session_;
			shared_ptr<RequestParserContext> context_;
			shared_ptr<Request> request_;
			bool request_completed_;
		};
	} // namespace http
} // namespace moss


