#include "application.h"

#include "route.h"
#include "routes.h"
#include "middleware.h"
#include "request.h"
#include "response.h"


namespace moss {
	namespace http {
		Application::Application()
			: routes_(std::make_shared<Routes>()) {
		}

		Application::Application(const string& prefix)
			: routes_(std::make_shared<Routes>()),
			prefix_(prefix) {
		}

		Application::~Application() {
		}

		string Application::Name() const {
			return "moss/1.0";
		}

		void Application::Install(shared_ptr<Route> route) {
			if (!route || route->Path().empty())
				return;
			route->AttachApplication(shared_from_this());
			routes_->Install(route);
		}

		void Application::Install(shared_ptr<Middleware> middleware) {
			if (!middleware)
				return;
			middleware->AttachApplication(shared_from_this());
			if (!middleware_) {
				middleware_ = middleware;
			} else {
				middleware_->Append(middleware);
			}
		}

		int Application::Process(shared_ptr<Request> request, shared_ptr<Response> response) {
			unordered_map<string, string> args;
			string route_path = request->Path();
			if (!prefix_.empty() && 0 == route_path.find(prefix_)) {
				route_path = route_path.substr(prefix_.length());
			}
			auto route = routes_->Find(request->Method(), route_path, args);
			if (!route)
				return -1;
			request->SetPathArgs(args);
			if (middleware_) {
				middleware_->Process(route, request, response);
			} else {
				response->SetStatusCode(200);
				route->Process(request, response);
			}
			return 0;
		}
	} // namespace http
} // namespace moss

