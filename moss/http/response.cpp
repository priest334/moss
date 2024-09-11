#include "response.h"

#include <ctime>
#include <sstream>
#include "third_party/http_parser/http_parser.h"
#include "internal/session.h"


using std::ostringstream;
namespace moss {
	namespace http {
		std::ostream& operator<<(std::ostream& stream, const Response& response) {
			stream << "HTTP/1.1 " << response.status_code_ << " " << http_status_str(http_status(response.status_code_)) << "\r\n";
			for (auto it = response.headers_.begin(); it != response.headers_.end(); ++it) {
				stream << it->first << ": " << it->second << "\r\n";
			}
			for (auto it = response.cookies_.begin(); it != response.cookies_.end(); ++it) {
				stream << "Set-Cookie: " << it->first << "=" << it->second << "\r\n";
			}
			stream << "\r\n";
			stream << response.payload_;
			return stream;
		}

		int Response::Send() {
			auto session = session_.lock();
			if (!session) {
				return -1;
			}
			ostringstream oss;
			oss << *this;
			auto wrbuf = std::make_shared<string>(std::move(oss.str()));
			return session->Write(wrbuf);
		}

		Response::Response(shared_ptr<Session> session)
			: session_(session),
			status_code_(404) {
		}

		shared_ptr<Session> Response::GetSession() const {
			return session_.lock();
		}

		void Response::SetStatusCode(int code) {
			status_code_ = code;
		}

		void Response::SetHeader(const string& key, const string& value) {
			headers_[key] = value;
		}

		void Response::SetPayload(const string& payload) {
			std::ostringstream oss;
			oss << payload.length();
			SetHeader("Content-Length", oss.str());
			payload_ = payload;
		}

		void Response::SetCookie(const string& name, const string& value) {
			cookies_[name] = value;
		}

		void Response::Redirect(const string& url) {
			SetHeader("Location", url);
			SetStatusCode(302);
		}

		string Response::Header(const string& key) const {
			string value;
			auto it = headers_.find(key);
			if (it != headers_.end()) {
				value = it->second;
			}
			return value;
		}
	} // namespace http
} // namespace moss

