#pragma once

#include <memory>
#include <string>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
using std::weak_ptr;
namespace moss {
	namespace http {
		class Application;
		class Request;
		class Response;
		class Route;

		class Middleware {
			friend class Application;
			enum class Stage {
				Before,
				After
			};
		public:
			MOSS_EXPORT Middleware(const string& name);
			MOSS_EXPORT virtual ~Middleware();
			MOSS_EXPORT string Name() const;
			shared_ptr<Application> CurrentApplication() const;
			void AttachApplication(shared_ptr<Application> application);
			void Append(shared_ptr<Middleware> middleware);
			MOSS_EXPORT virtual int OnBefore(shared_ptr<moss::http::Request> request, shared_ptr<moss::http::Response> response);
			MOSS_EXPORT virtual int OnAfter(shared_ptr<moss::http::Request> request, shared_ptr<moss::http::Response> response);
		protected:
			int Process(shared_ptr<Request> request, shared_ptr<Response> response, Stage stage);
			int Process(shared_ptr<Route> route, shared_ptr<Request> request, shared_ptr<Response> response);
			weak_ptr<Application> application_;
			shared_ptr<Middleware> sibling_;
			string name_;
		};
	} // namespace http
} // namespace moss


