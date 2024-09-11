#include "session.h"

#include "request_parser.h"
#include "../../tcp/connection.h"
#include "utils/logger.h"


namespace moss {
	namespace http {
		Session::Session(int64_t id, shared_ptr<Connection> connection, shared_ptr<HttpServerImpl> server)
			: id_(id),
			connection_(connection),
			server_(server),
			ip_(connection->Ip()),
			closing_(false),
			read_completed_(false),
			last_read_(0),
			last_write_(0) {
			ResetTimeout();
		}

		Session::~Session() {
		}

		int64_t Session::Id() const {
			return id_;
		}

		string Session::Ip() const {
			return ip_;
		}

		shared_ptr<Connection> Session::GetConnection() const {
			return connection_.lock();
		}

		void Session::Close() {
			closing_ = true;
			auto connection = connection_.lock();
			if (connection) {
				connection->Close();
			}
		}

		bool Session::IsClosing() const {
			return closing_;
		}

		void Session::ReadComplete() {
			read_completed_ = true;
		}

		bool Session::IsReadCompleted() const {
			return read_completed_;
		}

		void Session::LastRead(int64_t last_read) {
			last_read_ = last_read;
		}

		int64_t Session::LastRead() const {
			return last_read_;
		}

		void Session::LastWrite(int64_t last_write) {
			last_write_ = last_write;
		}

		int64_t Session::LastWrite() const {
			return last_write_;
		}

		void Session::ResetTimeout() {
			auto now = time(nullptr);
			last_read_ = now;
			last_write_ = now;
			read_completed_ = false;
		}

		int Session::CreateParser() {
			request_parser_ = std::make_shared<RequestParser>(shared_from_this());
			request_parser_->Initalize();
			return 0;
		}

		void Session::ResetParser() {
			if (!request_parser_) {
				CreateParser();
			} else {
				request_parser_->Reset();
			}
		}

		shared_ptr<Request> Session::Append(shared_ptr<string> rdbuf, size_t size) {
			if (size < 0)
				return nullptr;
			request_parser_->Parse(rdbuf->data(), (size_t)size);
			if (!request_parser_->IsRequestCompleted()) {
				return nullptr;
			}
			return request_parser_->GetRequest();
		}

		int Session::Write(shared_ptr<string> wrbuf) {
			auto connection = connection_.lock();
			if (connection) {
				connection->Write(wrbuf);
			}
			return 0;
		}
	} // namespace http
} // namespace moss

