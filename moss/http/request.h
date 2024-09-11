#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include "moss_exports.h"


using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::weak_ptr;
namespace moss {
	class Url;
	namespace http {
		class Session;
		class Request {
			using Headers = unordered_map<string, string>;
			using Cookies = unordered_map<string, string>;
			using PathArgs = unordered_map<string, string>;
		public:
			MOSS_EXPORT Request(shared_ptr<Session> session);
			MOSS_EXPORT shared_ptr<Session> GetSession() const;
			MOSS_EXPORT void SetMethod(const string& method);
			MOSS_EXPORT void SetUrl(const string& url);
			MOSS_EXPORT void SetHeader(const string& key, const string& value);
			MOSS_EXPORT void SetBody(const string& body);
			MOSS_EXPORT string Ip() const;
			MOSS_EXPORT string Method() const;
			MOSS_EXPORT string Url() const;
			MOSS_EXPORT string Path() const;
			MOSS_EXPORT string Query(const string& key) const;
			MOSS_EXPORT string Header(const string& key) const;
			MOSS_EXPORT string Body() const;
			MOSS_EXPORT string Cookie(const string& key) const;
			MOSS_EXPORT string ContentType() const;
			MOSS_EXPORT size_t ContentLength() const;
			MOSS_EXPORT void SetUserContext(shared_ptr<void> user_context);
			MOSS_EXPORT shared_ptr<void> UserContext();
			MOSS_EXPORT shared_ptr<void> UserContext() const;
			MOSS_EXPORT void SetPathArgs(PathArgs& args);
			MOSS_EXPORT string Path(const string& key);
		private:
			weak_ptr<Session> session_;
			string method_;
			string body_;
			string ip_;
			shared_ptr<moss::Url> url_;
			Headers headers_;
			Cookies cookies_;
			shared_ptr<void> user_context_;
			PathArgs path_args_;
		};
	} // namespace http
} // namespace moss

