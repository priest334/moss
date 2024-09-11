#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::weak_ptr;
namespace moss {
	class HttpServer;
	namespace http {
		class Session;
		class Response
			: public std::enable_shared_from_this<Response> {
			using Headers = unordered_map<string, string>;
			using Cookies = unordered_map<string, string>;
			friend std::ostream& operator<<(std::ostream& stream, const Response& response);
			friend class moss::HttpServer;
			int Send();
		public:
			MOSS_EXPORT Response(shared_ptr<Session> session);
			shared_ptr<Session> GetSession() const;
			MOSS_EXPORT void SetStatusCode(int code);
			MOSS_EXPORT void SetHeader(const string& key, const string& value);
			MOSS_EXPORT void SetPayload(const string& payload);
			MOSS_EXPORT void SetCookie(const string& key, const string& value);
			MOSS_EXPORT void Redirect(const string& url);
			MOSS_EXPORT string Header(const string& key) const;
		private:
			weak_ptr<Session> session_;
			int status_code_;
			Headers headers_;
			Cookies cookies_;
			string payload_;
		};
	} // namespace http
} // namespace moss


