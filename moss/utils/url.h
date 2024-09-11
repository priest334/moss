#pragma once

#include <string>
#include <unordered_map>


using std::string;
using std::unordered_map;
namespace moss {
	class Url {
		using Parameters = unordered_map<string, string>;
	public:
		static string Encode(const string& value);
		static string Decode(const string& value);
		Url();
		Url(const string& url);
		bool Parse(const string& url);
		string Get(const string& key, const string& default_value) const;
		string Get(const string& key) const;
		string Query(const string& key, const string& default_value) const;
		string Query(const string& key) const;
		Url& Set(const string& key, const string& value);
		Url& SetQuery();
		Url& SetQuery(const string& key, const string& value);
		Url& SetQueries(const Parameters& queries);
		Url& SetQueryString(const string& query_string);
		string Make() const;
		string Schema() const;
		string UserInfo() const;
		string Host() const;
		string HttpHost() const;
		string Port() const;
		string Path() const;
		string Fragment() const;
	private:
		Parameters parameters_;
		Parameters queries_;
	};
} // namespace moss

