#include "url.h"

#include <sstream>
#include "utils/string_helper.h"
#include "third_party/http_parser/http_parser.h"


using std::ostringstream;
namespace moss {
	namespace {
		const int ishexchar[] = {
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,
			0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		};
		const int hexval[] = {
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
			0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0xa,0xb,0xc,0xd,0xe,0xf,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
		};
		const char* urlchar[] = {
			"%00", "%01", "%02", "%03", "%04", "%05", "%06", "%07", "%08", "%09", "%0A", "%0B", "%0C", "%0D", "%0E", "%0F",
			"%10", "%11", "%12", "%13", "%14", "%15", "%16", "%17", "%18", "%19", "%1A", "%1B", "%1C", "%1D", "%1E", "%1F",
			"%20", "!", "%22", "%23", "%24", "%25", "%26", "%27", "(", ")", "*", "%2B", "%2C", "%2D", ".", "%2F",
			"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "%3A", "%3B", "%3C", "%3D", "%3E", "%3F",
			"%40", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
			"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "%5B", "%5C", "%5D", "%5E", "_",
			"%60", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o",
			"p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "%7B", "%7C", "%7D", "~", "%7F",
			"%80", "%81", "%82", "%83", "%84", "%85", "%86", "%87", "%88", "%89", "%8A", "%8B", "%8C", "%8D", "%8E", "%8F",
			"%90", "%91", "%92", "%93", "%94", "%95", "%96", "%97", "%98", "%99", "%9A", "%9B", "%9C", "%9D", "%9E", "%9F",
			"%A0", "%A1", "%A2", "%A3", "%A4", "%A5", "%A6", "%A7", "%A8", "%A9", "%AA", "%AB", "%AC", "%AD", "%AE", "%AF",
			"%B0", "%B1", "%B2", "%B3", "%B4", "%B5", "%B6", "%B7", "%B8", "%B9", "%BA", "%BB", "%BC", "%BD", "%BE", "%BF",
			"%C0", "%C1", "%C2", "%C3", "%C4", "%C5", "%C6", "%C7", "%C8", "%C9", "%CA", "%CB", "%CC", "%CD", "%CE", "%CF",
			"%D0", "%D1", "%D2", "%D3", "%D4", "%D5", "%D6", "%D7", "%D8", "%D9", "%DA", "%DB", "%DC", "%DD", "%DE", "%DF",
			"%E0", "%E1", "%E2", "%E3", "%E4", "%E5", "%E6", "%E7", "%E8", "%E9", "%EA", "%EB", "%EC", "%ED", "%EE", "%EF",
			"%F0", "%F1", "%F2", "%F3", "%F4", "%F5", "%F6", "%F7", "%F8", "%F9", "%FA", "%FB", "%FC", "%FD", "%FE", "%FF"
		};

		class UrlParser {
		public:
			UrlParser() {
				http_parser_url_init(&parser_);
			}

			UrlParser(const string& url)
				: url_(url) {
				http_parser_url_init(&parser_);
				Parse(url_);
			}

			UrlParser& Parse(const string& url) {
				url_ = url;
				http_parser_parse_url(url.c_str(), url.length(), false, &parser_);
				return *this;
			}

			string Field(int field) {
				return url_.substr(parser_.field_data[field].off, parser_.field_data[field].len);
			}

		private:
			string url_;
			http_parser_url parser_;
		};
	}

	string Url::Encode(const string& value) {
		string retval;
		size_t len = value.length();
		for (size_t i = 0; i < len; i++) {
			retval.append(urlchar[(unsigned char)value[i]]);
		}
		return retval;
	}

	string Url::Decode(const string& value) {
		string retval;
		size_t i = 0, j = 0, len = value.length();
		for (; i < len; j++) {
			char c = value[i];
			if (c == '%' && (i + 2) < len && 1 == ishexchar[value[i + 1]] && 1 == ishexchar[value[i + 2]]) {
				char ch = ((hexval[value[i + 1]] << 4) & 0xf0) | (hexval[value[i + 2]] & 0x0f);
				retval.append(1, ch);
				i += 3;
			} else {
				retval.append(1, c);
				i++;
			}
		}
		return retval;
	}

	Url::Url() {
	}

	Url::Url(const string& url) {
		Parse(url);
	}

	bool Url::Parse(const string& url) {
		if (url.empty())
			return false;
		UrlParser parser(url);
		Set("schema", parser.Field(UF_SCHEMA));
		Set("user_info", parser.Field(UF_USERINFO));
		Set("host", parser.Field(UF_HOST));
		Set("port", parser.Field(UF_PORT));
		Set("path", parser.Field(UF_PATH));
		Set("fragment", parser.Field(UF_FRAGMENT));
		string query = parser.Field(UF_QUERY);
		Set("query", query);
		if (!query.empty()) {
			queries_ = String(query).split("&", "=");
		}
		return true;
	}

	string Url::Get(const string& key, const string& default_value) const {
		auto it = parameters_.find(key);
		if (it != parameters_.end()) {
			return it->second;
		} else {
			return Query(key, default_value);
		}
	}

	string Url::Get(const string& key) const {
		return Get(key, string());
	}

	string Url::Query(const string& key, const string& default_value) const {
		auto it = queries_.find(key);
		if (it != queries_.end()) {
			return Decode(it->second);
		}
		return default_value;
	}

	string Url::Query(const string& key) const {
		return Query(key, string());
	}

	Url& Url::Set(const string& key, const string& value) {
		parameters_[key] = value;
		return *this;
	}

	Url& Url::SetQuery() {
		auto query = String("&").Join(queries_, "=", [](const unordered_map<string,string>::value_type& query) {
			return query.first + "=" + Encode(query.second);
		});
		Set("query", query);
		return *this;
	}

	Url& Url::SetQuery(const string& key, const string& value) {
		queries_[key] = value;
		return SetQuery();
	}

	Url& Url::SetQueries(const Parameters& queries) {
		for (auto it : queries) {
			queries_[it.first] = it.second;
		}
		return SetQuery();
	}

	Url& Url::SetQueryString(const string& query_string) {
		SetQueries(String(query_string).split("&", "="));
		Set("query", query_string);
		return *this;
	}

	string Url::Make() const {
		string user_info = UserInfo();
		string port = Port();
		string path = Path();
		string fragment = Fragment();
		string query = Get("query");
		string schema = Schema();
		ostringstream oss;
		if (!schema.empty())
			oss << Schema() << "://";
		if (!user_info.empty())
			oss << user_info << "@";
		oss << Host();
		if (!port.empty()) {
			oss << ":" << port;
		}
		oss << Path();
		if (!query.empty()) {
			oss << "?" << query;
		}
		if (!fragment.empty()) {
			oss << "#" << fragment;
		}
		return oss.str();
	}

	string Url::Schema() const {
		return Get("schema", "http");
	}

	string Url::UserInfo() const {
		return Get("user_info");
	}

	string Url::Host() const {
		return Get("host");
	}

	string Url::HttpHost() const {
		string port = Port();
		if (port.empty())
			return Host();
		return Host() + ":" + port;
	}

	string Url::Port() const {
		return Get("port");
	}

	string Url::Path() const {
		return Get("path", "/");
	}

	string Url::Fragment() const {
		return Get("fragment");
	}		
} // namespace moss

