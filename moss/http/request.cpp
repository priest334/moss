#include "request.h"

#include <iostream>
#include <sstream>
#include "internal/session.h"
#include "utils/string_helper.h"
#include "utils/url.h"


namespace moss {
	namespace http {
		Request::Request(shared_ptr<Session> session)
			: session_(session) {
		}

		shared_ptr<Session> Request::GetSession() const {
			return session_.lock();
		}

		void Request::SetMethod(const string& method) {
			method_ = method;
		}

		void Request::SetUrl(const string& url) {
			url_ = std::make_shared<moss::Url>(url);
		}

		void Request::SetHeader(const string& key, const string& value) {
			string header = String(key).ToLower().str();
			headers_[header] = value;
			if (header == "cookie") {
				cookies_ = String(value).split(";", "=");
			} else if (header == "host" || header == "x-forwarded-host") {
				url_->Set("host", value);
			} else if (header == "x-schema" || header == "x-forwarded-proto") {
				url_->Set("schema", value);
			} else if (header == "x-real-ip") {
				ip_ = value;
			}
		}

		void Request::SetBody(const string& body) {
			body_ += body;
		}

		string Request::Ip() const {
			auto session = session_.lock();
			if (!ip_.empty() || !session) {
				return ip_;
			}
			return session->Ip();
		}

		string Request::Method() const {
			return method_;
		}

		string Request::Url() const {
			return url_->Make();
		}

		string Request::Path() const {
			return url_->Path();
		}

		string Request::Query(const string& key) const {
			return url_->Query(key);
		}

		string Request::Header(const string& key) const {
			auto it = headers_.find(String(key).ToLower());
			if (it != headers_.end()) {
				return it->second;
			}
			return string();
		}

		string Request::Body() const {
			return body_;
		}

		string Request::Cookie(const string& key) const {
			auto it = cookies_.find(key);
			if (it != cookies_.end()) {
				return it->second;
			}
			return string();
		}

		string Request::ContentType() const {
			return Header("Content-Type");
		}

		size_t Request::ContentLength() const {
			string content_length = Header("Content-Length");
			return std::strtoul(content_length.c_str(), nullptr, 10);
		}

		void Request::SetUserContext(shared_ptr<void> user_context) {
			user_context_ = user_context;
		}

		shared_ptr<void> Request::UserContext() {
			return user_context_;
		}

		shared_ptr<void> Request::UserContext() const {
			return user_context_;
		}

		void Request::SetPathArgs(PathArgs& args) {
			path_args_.swap(args);
		}

		string Request::Path(const string& key) {
			auto it = path_args_.find(key);
			if (it != path_args_.end()) {
				return it->second;
			}
			return string();
		}
	} // namespace http
} // namespace moss

