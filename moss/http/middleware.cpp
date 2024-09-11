#include "middleware.h"

#include "route.h"
#include "request.h"
#include "response.h"


namespace moss {
	namespace http {
		Middleware::Middleware(const string& name)
			: name_(name) {
		}

		Middleware::~Middleware() {
		}

		string Middleware::Name() const {
			return name_;
		}

		shared_ptr<Application> Middleware::CurrentApplication() const {
			return application_.lock();
		}

		void Middleware::AttachApplication(shared_ptr<Application> application) {
			application_ = application;
		}

		void Middleware::Append(shared_ptr<Middleware> middleware) {
			if (sibling_) {
				sibling_->Append(middleware);
			} else {
				sibling_ = middleware;
			}
		}

		int Middleware::OnBefore(shared_ptr<moss::http::Request> request, shared_ptr<moss::http::Response> response) {
			return 0;
		}
		
		int Middleware::OnAfter(shared_ptr<moss::http::Request> request, shared_ptr<moss::http::Response> response) {
			return 0;
		}

		int Middleware::Process(shared_ptr<Request> request, shared_ptr<Response> response, Stage stage) {
			if (stage == moss::http::Middleware::Stage::Before) {
				return OnBefore(request, response);
			} else {
				return OnAfter(request, response);
			}
		}

		int Middleware::Process(shared_ptr<Route> route, shared_ptr<Request> request, shared_ptr<Response> response) {
			int code = Process(request, response, Stage::Before);
			if (code < 0) {
				return code;
			}
			if (sibling_) {
				code = sibling_->Process(route, request, response);
			} else if (route) {
				response->SetStatusCode(200);
				code = route->Process(request, response);
			}	
			return Process(request, response, Stage::After);
		}
	} // namespace http
} // namespace moss

