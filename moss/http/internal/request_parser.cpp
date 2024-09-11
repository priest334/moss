#include "request_parser.h"

#include "../request.h"
#include "third_party/http_parser/http_parser.h"


namespace moss {
	namespace http {
		class RequestParserContext {
			friend class RequestParser;
			static shared_ptr<RequestParser> get_request_parser(http_parser* parser) {
				RequestParserContext* p = reinterpret_cast<RequestParserContext*>(parser->data);
				auto request_parser = p->GetRequestParser();
				return request_parser;
			}

			static shared_ptr<Request> get_current_request(http_parser* parser) {
				RequestParserContext* p = reinterpret_cast<RequestParserContext*>(parser->data);
				auto request_parser = p->GetRequestParser();
				if (request_parser) {
					return request_parser->GetRequest();
				}
				return nullptr;
			}

			static int on_url(http_parser* parser, const char* at, size_t length) {
				auto request = get_current_request(parser);
				request->SetUrl(string(at, length));
				return 0;
			}

			static int on_headers_complete(http_parser* parser) {
				return 0;
			}

			static int on_body(http_parser* parser, const char* at, size_t length) {
				auto request = get_current_request(parser);
				request->SetBody(string(at, length));
				return 0;
			}

			static int on_header_field(http_parser* parser, const char* at, size_t length) {
				RequestParserContext* p = reinterpret_cast<RequestParserContext*>(parser->data);
				p->current_header_ = string(at, length);
				return 0;
			}

			static int on_header_value(http_parser* parser, const char* at, size_t length) {
				RequestParserContext* p = reinterpret_cast<RequestParserContext*>(parser->data);
				auto request = get_current_request(parser);
				request->SetHeader(p->current_header_, string(at, length));
				return 0;
			}

			static int on_message_begin(http_parser* parser) {
				auto request_parser = get_request_parser(parser);
				request_parser->PrepareRequest();
				auto request = request_parser->GetRequest();
				request->SetMethod(http_method_str((http_method)parser->method));
				return 0;
			}

			static int on_message_complete(http_parser* parser) {
				auto request_parser = get_request_parser(parser);
				request_parser->CompleteRequest();
				return 0;
			}

			static int on_chunk_header(http_parser* parser) {
				auto request_parser = get_current_request(parser);
				return 0;
			}

			static int on_chunk_complete(http_parser* parser) {
				auto request_parser = get_current_request(parser);
				return 0;
			}
		public:
			RequestParserContext(shared_ptr<RequestParser> request_parser)
				: request_parser_(request_parser),
				settings_(std::make_shared<http_parser_settings>()),
				parser_(std::make_shared<http_parser>()) {
				http_parser_settings_init(settings_.get());
				settings_->on_message_begin = on_message_begin;
				settings_->on_url = on_url;
				settings_->on_header_field = on_header_field;
				settings_->on_header_value = on_header_value;
				settings_->on_headers_complete = on_headers_complete;
				settings_->on_body = on_body;
				settings_->on_message_complete = on_message_complete;
				settings_->on_chunk_header = on_chunk_header;
				settings_->on_chunk_complete = on_chunk_complete;
				http_parser_init(parser_.get(), HTTP_REQUEST);
				parser_->data = this;
			}

			void Reset() {
				http_parser_init(parser_.get(), HTTP_REQUEST);
				parser_->data = this;
			}

			size_t Parse(const char* data, size_t len) {
				return http_parser_execute(parser_.get(), settings_.get(), data, len);
			}

			shared_ptr<RequestParser> GetRequestParser() {
				return request_parser_.lock();
			}
		private:
			weak_ptr<RequestParser> request_parser_;
			shared_ptr<http_parser_settings> settings_;
			shared_ptr<http_parser> parser_;
			string current_header_;
		};

		RequestParser::RequestParser(shared_ptr<Session> session)
			: session_(session),
			request_(std::make_shared<Request>(session)),
			request_completed_(false) {
		}

		void RequestParser::Initalize() {
			context_ = std::make_shared<RequestParserContext>(shared_from_this());
		}

		void RequestParser::Reset() {
			context_->Reset();
		}

		size_t RequestParser::Parse(const char* data, size_t len) {
			return context_->Parse(data, len);
		}

		void RequestParser::PrepareRequest() {
			request_completed_ = false;
			request_ = std::make_shared<Request>(session_.lock());
		}

		void RequestParser::CompleteRequest() {
			request_completed_ = true;
		}

		bool RequestParser::IsRequestCompleted() const {
			return request_completed_;
		}

		shared_ptr<Request> RequestParser::GetRequest() {
			return request_;
		}
	} // namespace http
} // namespace moss

