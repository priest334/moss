#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "application.h"
#include "request.h"
#include "response.h"
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
using std::unordered_map;
namespace moss {
	class HttpServerImpl;
	class HttpServer
		: public std::enable_shared_from_this<HttpServer> {
		using Applications = unordered_map<string, shared_ptr<http::Application>>;
		friend class HttpServerImpl;
	public:
		MOSS_EXPORT HttpServer();
		MOSS_EXPORT int Install(shared_ptr<http::Application> application);
		MOSS_EXPORT int Start(const string& ip, int port, int workers = 10);
		MOSS_EXPORT int Stop();
	protected:
		int Process(shared_ptr<http::Request> request, shared_ptr<http::Response> response);
	private:
		shared_ptr<HttpServerImpl> impl_;
		Applications applications_;
	};
} // namespace moss

