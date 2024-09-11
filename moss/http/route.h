#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::weak_ptr;
namespace moss {
	namespace http {
		class Application;
		class Request;
		class Response;

		class Route {
			friend class Routes;
			friend class Application;
			void Append(shared_ptr<Route> sibling);
			shared_ptr<Route> Match(const string& method);
		public:
			MOSS_EXPORT Route(const string& method, const string& path);
			MOSS_EXPORT virtual ~Route();
			MOSS_EXPORT shared_ptr<Application> CurrentApplication() const;
			MOSS_EXPORT void AttachApplication(shared_ptr<Application> application);
			MOSS_EXPORT virtual bool MatchMethod(const string& method) const;
			MOSS_EXPORT virtual bool Match(const string& method, const string& pattern, const string& path, unordered_map<string, string>& args) const;
			MOSS_EXPORT virtual string Method() const;
			MOSS_EXPORT virtual string Path() const;
			MOSS_EXPORT virtual int Process(shared_ptr<Request> request, shared_ptr<Response> response) = 0;
		protected:
			weak_ptr<Application> application_;
			shared_ptr<Route> sibling_;
			string method_;
			string path_;
		};
	} // namespace http	
} // namespace moss


