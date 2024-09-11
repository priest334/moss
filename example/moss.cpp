#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include "http/application.h"
#include "http/http_server.h"
#include "http/route.h"
#include "http/request.h"
#include "http/response.h"
#include "http/middleware.h"


using namespace std;


class Banner
	: public moss::http::Middleware {
public:
	Banner()
		: moss::http::Middleware("banner") {
	}

	int OnBefore(shared_ptr<moss::http::Request> request, shared_ptr<moss::http::Response> response) override {
		string name = request->Query("name");
		if (name == "badguys") {
			response->SetStatusCode(403);
			response->SetPayload("banner " + name);
			return -1;
		}
		return 0;
	}
};

class Hello
	: public moss::http::Route {
public:
	Hello()
		: moss::http::Route("GET", "/hello") {
	}

	int Process(shared_ptr<moss::http::Request> request, shared_ptr<moss::http::Response> response) override {
		string name = request->Query("name");
		if (name.empty())
			name = "moss";
		response->SetPayload("hello " + name);
		return 0;
	}
};


int main(int argc, char* argv[]) {
	auto application = std::make_shared<moss::http::Application>("/moss");
	application->Install(std::make_shared<Banner>());
	application->Install(std::make_shared<Hello>());
	auto server = std::make_shared<moss::HttpServer>();
	server->Install(application);
	int code = server->Start("127.0.0.1", 2024);
	while (!code) {
		std::this_thread::sleep_for(std::chrono::seconds(10));
	}
	return 0;
}
