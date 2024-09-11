#include "http_server.h"

#include "application.h"
#include "request.h"
#include "response.h"
#include "internal/http_server_impl.h"


namespace moss {
	HttpServer::HttpServer() {
	}

	int HttpServer::Install(shared_ptr<http::Application> application) {
		if (!application)
			return -1;
		applications_[application->Name()] = application;
		return 0;
	}

	int HttpServer::Start(const string& ip, int port, int workers/* = 10*/) {
		impl_ = std::make_shared<HttpServerImpl>(shared_from_this());
		return impl_->Start(ip, port, workers);
	}

	int HttpServer::Stop() {
		return impl_->Stop();
	}

	int HttpServer::Process(shared_ptr<http::Request> request, shared_ptr<http::Response> response) {
		shared_ptr<http::Application> application;
		for (auto& it : applications_) {
			int handled = it.second->Process(request, response);
			if (handled < 0)
				continue;
			application = it.second;
			break;
		}
		auto connection_header = request->Header("Connection");
		auto server_header = response->Header("Server");
		if (server_header.empty()) {
			if (application) {
				response->SetHeader("Server", application->Name());
			} else {
				response->SetHeader("Server", "moss");
			}
		}
		if (!connection_header.empty()) {
			response->SetHeader("Connection", connection_header);
		} else {
			response->SetHeader("Connection", "close");
		}
		response->Send();
		return 0;
	}
} // namespace moss


