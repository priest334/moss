#include "route.h"

#include <regex>
#include <vector>
#include <sstream>
#include "utils/logger.h"


using std::vector;
namespace moss {
	namespace {
		bool MatchPathArgs(const string& pattern, const string& path, unordered_map<string, string>& args) {
			string expr;
			std::vector<string> keys;
			int group = 0;
			std::ostringstream oss;
			for (size_t i = 0; i < pattern.length(); i++) {
				char ch = pattern.at(i);
				if (ch == '{') {
					size_t j = i + 1, k = 0, n = 0;
					for (; j < pattern.length(); j++) {
						char ch1 = pattern.at(j);
						if (ch1 == '{') {
							n++;
						} else if (ch1 == '}') {
							if (n == 0) {
								if (k > 0 && k < j) {
									string value = pattern.substr(k, j - k);
									expr.append("(" + value + ")");
								} else {
									string name = pattern.substr(i + 1, j - i - 1);
									keys.push_back(name);
									expr.append("([^/\\?]+)");
								}
								i = j;
								break;
							}
							n--;
						}
						if (ch1 == ':') {
							string name = pattern.substr(i + 1, j - i - 1);
							keys.push_back(name);
							k = j + 1;
						}
					}
				} else {
					if (ch == '(') {
						oss.str("");
						oss << "g" << ++group;
						keys.push_back(oss.str());
					}
					expr.append(1, ch);
				}
			}
			try {
				std::cmatch mr;
				std::regex express(expr);
				if (std::regex_match(path.c_str(), mr, express)) {
					for (size_t i = 0; i < keys.size(); i++) {
						args[keys[i]] = mr[i + 1];
					}
					return true;
				}
			} catch (const std::regex_error& e) {
				logger::Error() << e.what();
			}
			return false;
		}
	}
	namespace http {
		void Route::Append(shared_ptr<Route> sibling) {
			if (sibling_) {
				sibling_->Append(sibling);
			} else {
				sibling_ = sibling;
			}
		}

		shared_ptr<Route> Route::Match(const string& method) {
			if (!sibling_)
				return nullptr;
			if (sibling_->MatchMethod(method)) {
				return sibling_;
			}
			return sibling_->Match(method);
		}

		Route::Route(const string& method, const string& path)
			: method_(method.c_str()), path_(path.c_str()) {
		}

		Route::~Route() {
		}

		shared_ptr<Application> Route::CurrentApplication() const {
			return application_.lock();
		}

		void Route::AttachApplication(shared_ptr<Application> application) {
			application_ = application;
		}

		bool Route::MatchMethod(const string& method) const {
			if (method_.empty() || method_ == "*")
				return true;
			return string::npos != method_.find(method);
		}

		bool Route::Match(const string& method, const string& pattern, const string& path, unordered_map<string, string>& args) const {
			if (MatchMethod(method) && MatchPathArgs(pattern, path, args)) {
				return true;
			}
			return false;
		}

		string Route::Method() const {
			return method_;
		}

		string Route::Path() const {
			return path_;
		}

	} // namespace http
} // namespace moss

