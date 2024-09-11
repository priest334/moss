#include "http_server_impl.h"

#include "session.h"
#include "../application.h"
#include "../request.h"
#include "../response.h"
#include "../http_server.h"
#include "../../tcp/connection.h"
#include "../../tcp/tcp_server.h"
#include "utils/logger.h"
#include "utils/task.h"
#include "utils/timer.h"
#include "utils/string_helper.h"


namespace moss {
	namespace {
		class RequestTimeoutChecker
			: public moss::Task {
		public:
			RequestTimeoutChecker(shared_ptr<HttpServerImpl> server)
				: server_(server) {
			}

			void Run() override {
				auto server = server_.lock();
				if (server) {
					server->CheckRequestTimeout();
				}
			}
		private:
			weak_ptr<HttpServerImpl> server_;
		};

		class RequestHandler
			: public moss::Task {
		public:
			RequestHandler(shared_ptr<HttpServerImpl> server, shared_ptr<http::Request> request, shared_ptr<http::Response> response)
				: server_(server), request_(request), response_(response) {
			}

			void Run() override {
				auto server = server_.lock();
				if (server) {
					server->Process(request_, response_);
				}
			}
		private:
			weak_ptr<HttpServerImpl> server_;
			shared_ptr<http::Request> request_;
			shared_ptr<http::Response> response_;
		};
	}

	HttpServerImpl::HttpServerImpl(weak_ptr<HttpServer> context)
		: context_(context),
		session_id_seq_(0),
		mutex_(std::make_shared<mutex>()),
		read_timeout_(15),
		write_timeout_(30) {
	}

	int HttpServerImpl::Start(const string& ip, int port, int workers) {
		server_ = std::make_shared<TcpServer>(shared_from_this());
		task_runner_ = std::make_shared<TaskRunner>();
		timer_loop_ = std::make_shared<TimerLoop>();
		task_runner_->Start(workers);
		timer_loop_->Start();
		auto timer = std::make_shared<Timer>(std::chrono::seconds(1));
		timer->Start(std::make_shared<RequestTimeoutChecker>(shared_from_this()));
		return server_->Start(ip, port);
	}

	int HttpServerImpl::Stop() {
		return server_->Stop();
	}

	int HttpServerImpl::OnCreate(shared_ptr<Connection> connection) {
		auto session = std::make_shared<http::Session>(session_id_seq_.fetch_add(1), connection, shared_from_this());
		session->CreateParser();
		connection->SetUserContext(session);
		std::lock_guard<mutex> lock(*mutex_);
		sessions_[session->Id()] = session;
		return 0;
	}

	int HttpServerImpl::OnRead(shared_ptr<Connection> connection, shared_ptr<string> rdbuf, int size) {
		auto session = std::static_pointer_cast<http::Session>(connection->UserContext());
		if (!session)
			return -1;
		session->LastRead((int64_t)time(nullptr));
		auto request = session->Append(rdbuf, size);
		if (request) {
			session->ReadComplete();
			session->ResetParser();
			auto response = std::make_shared<http::Response>(session);
			task_runner_->Push(std::make_shared<RequestHandler>(shared_from_this(), request, response));
		}
		return 0;
	}

	int HttpServerImpl::OnWrite(shared_ptr<moss::Connection> connection, shared_ptr<string> wrbuf, int status) {
		auto session = std::static_pointer_cast<http::Session>(connection->UserContext());
		if (!session)
			return -1;
		session->LastWrite((int64_t)time(nullptr));
		session->Close();
		return 0;
	}

	int HttpServerImpl::OnClose(shared_ptr<moss::Connection> connection) {
		auto session = std::static_pointer_cast<http::Session>(connection->UserContext());
		if (!session)
			return -1;
		std::lock_guard<mutex> lock(*mutex_);
		auto it = sessions_.find(session->Id());
		if (it != sessions_.end()) {
			sessions_.erase(it);
		}
		return 0;
	}

	int HttpServerImpl::OnError(int64_t id, const string& message) {
		logger::Fatal() << "http server error: " << id << ", " << message;
		return 0;
	}

	int HttpServerImpl::CheckRequestTimeout() {
		Sessions sessions;
		{
			std::lock_guard<mutex> lock(*mutex_);
			sessions = sessions_;
		}
		time_t now = time(nullptr);
		for (auto& it : sessions) {
			auto session = it.second;
			if (!session || session->IsClosing())
				continue;
			bool timeout = false;
			if (!session->IsReadCompleted()) {
				if (now - session->LastRead() > read_timeout_) {
					timeout = true;
				}
			} else {
				if (now - session->LastWrite() > write_timeout_) {
					timeout = true;
				}
			}
			if (timeout) {
				session->Close();
			}
		}
		return 0;
	}

	int HttpServerImpl::Process(shared_ptr<http::Request> request, shared_ptr<http::Response> response) {
		auto context = context_.lock();
		return context->Process(request, response);
	}

	int HttpServerImpl::CloseSession(shared_ptr<http::Session> session) {
		if (!session)
			return -1;
		session->Close();
		std::lock_guard<mutex> lock(*mutex_);
		auto it = sessions_.find(session->Id());
		if (it != sessions_.end()) {
			sessions_.erase(it);
		}
		return 0;
	}
} // namespace moss


