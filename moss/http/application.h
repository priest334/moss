#pragma once

#include <memory>
#include <string>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
namespace moss {
	class HttpServer;
	namespace http {
		class Route;
		class Routes;
		class Middleware;
		class Request;
		class Response;

		class Application
			: public std::enable_shared_from_this<Application> {
			friend class moss::HttpServer;
		public:
			MOSS_EXPORT Application();
			MOSS_EXPORT Application(const string& prefix);
			virtual ~Application();
			MOSS_EXPORT virtual string Name() const;
			MOSS_EXPORT void Install(shared_ptr<Route> route);
			MOSS_EXPORT void Install(shared_ptr<Middleware> middleware);
		protected:
			int Process(shared_ptr<Request> request, shared_ptr<Response> response);
		private:
			shared_ptr<Routes> routes_;
			shared_ptr<Middleware> middleware_;
			string prefix_;
		};
	} // namespace http
} // namespace moss


