#pragma once

#include <cstdarg>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>


using std::map;
using std::string;
using std::unordered_map;
using std::vector;
using std::ostringstream;
namespace moss {
	class String {
	public:
		static String Digits;
		static String CapitalLetters;
		static String SmallLetters;
		static String Letters;
		static String Normal;
		static String Printable;
		static String WhiteSpace;
		static String Random(size_t length, const String& chars);
		static String Random(size_t length);

		String();
		String(size_t size, char ch = '\0');
		String(const char* str);
		String(const char* str, size_t length);
		String(const string& str);
		String(const String& str);
		String& Format(const char* fmt, ...);
		String& Assign(const char* str);
		String& Assign(const char* str, size_t length);
		String& Assign(const string& str);
		String& Assign(const String& str);
		String& Append(const char* str);
		String& Append(const char* str, size_t length);
		String& Append(const string& str);
		String& Append(const String& str);
		String Center(size_t length, char fill = ' ') const;
		String StripLeft(const String& chars) const;
		String StripLeft() const;
		String StripRight(const String& chars) const;
		String StripRight() const;
		String Strip(const String& chars) const;
		String Strip() const;
		String SubStr(int start, int end = -1) const;
		String RemovePrefix(const String& str) const;
		String RemoveSuffix(const String& str) const;
		String Replace(const String& old, const String& str) const;
		vector<string> split(const string& seperator, size_t maxsplit = -1) const;
		vector<String> Split(const String& seperator, size_t maxsplit = -1) const;
		unordered_map<string, string> split(const string& seperator, const string& assign_key) const;
		unordered_map<String, String> Split(const String& seperator, const String& assign_key) const;
		String CamelToUnderscore() const;
		String UnderscoreToCamel(bool little = true) const;
		String Translate(const unordered_map<char, char>& table) const;
		String ToUpper() const;
		String ToLower() const;
		//String Utf8ToGbk() const;
		//String GbkToUtf8() const;
		
		bool IsDigit() const;
		bool IsNumeric() const;
		bool IsHex() const;
		bool StartsWith(const String& prefix) const;
		bool EndsWith(const String& suffix) const;
		int Find(const String& sub) const;
		int ToInt() const;
		int64_t ToInt64() const;
		float ToFloat() const;
		double ToDouble() const;
		String& resize(size_t new_size, const char fill = (char)0);
		char& operator[](int index);
		char operator[](int index) const;
		string str() const;
		char const * c_str() const;
		size_t length() const;
		size_t size() const;
		operator int() const;
		operator char const* () const;
		operator string const& () const;

		String& operator=(const char* str);
		String& operator=(const string& str);
		String& operator=(const String& str);
		String& operator+=(const char* str);
		String& operator+=(const string& str);
		String& operator+=(const String& str);
		friend bool operator<(const char* left, const String& right);
		friend bool operator<(const String& left, const char* right);
		friend bool operator<(const string& left, const String& right);
		friend bool operator<(const String& left, const string& right);
		friend bool operator<(const String& left, const String& right);
		friend bool operator==(const char* left, const String& right);
		friend bool operator==(const String& left, const char* right);
		friend bool operator==(const string& left, const String& right);
		friend bool operator==(const String& left, const string& right);
		friend bool operator==(const String& left, const String& right);
		friend bool operator>(const char* left, const String& right);
		friend bool operator>(const String& left, const char* right);
		friend bool operator>(const string& left, const String& right);
		friend bool operator>(const String& left, const string& right);
		friend bool operator>(const String& left, const String& right);
		friend bool operator!=(const char* left, const String& right);
		friend bool operator!=(const String& left, const char* right);
		friend bool operator!=(const string& left, const String& right);
		friend bool operator!=(const String& left, const string& right);
		friend bool operator!=(const String& left, const String& right);
		friend bool operator<=(const char* left, const String& right);
		friend bool operator<=(const String& left, const char* right);
		friend bool operator<=(const string& left, const String& right);
		friend bool operator<=(const String& left, const string& right);
		friend bool operator<=(const String& left, const String& right);
		friend bool operator>=(const char* left, const String& right);
		friend bool operator>=(const String& left, const char* right);
		friend bool operator>=(const string& left, const String& right);
		friend bool operator>=(const String& left, const string& right);
		friend bool operator>=(const String& left, const String& right);
		friend std::ostream& operator<<(std::ostream& stream, const String& str);


		template<class Container>
		String Join(const Container& values) const {
			if (values.empty())
				return String();
			ostringstream oss;
			auto it = values.begin();
			for (;;) {
				oss << *it;
				if (++it == values.end())
					break;
				oss << value_;
			}
			return String(oss.str());
		}

		template<class Container>
		String Join(const Container& values, string(*handler)(const typename Container::value_type&)) const {
			if (values.empty())
				return String();
			ostringstream oss;
			auto it = values.begin();
			for (;;) {
				if (!handler)
					continue;
				oss << handler(*it);
				if (++it == values.end())
					break;
				oss << value_;
			}
			return String(oss.str());
		}

		template<class Container>
		String Join(const Container& values, const string& assign_key) const {
			if (values.empty())
				return String();
			bool first = true;
			ostringstream oss;
			for (auto& it : values) {
				if (!first) {
					oss << value_;
				} else {
					first = !first;
				}
				oss << it.first << assign_key << it.second;
			}
			return String(oss.str());
		}

		template<class Container>
		String Join(const Container& values, const string& assign_key, string(*handler)(const typename Container::value_type&)) const {
			if (values.empty())
				return String();
			ostringstream oss;
			auto it = values.begin();
			for (;;) {
				if (!handler)
					continue;
				oss << handler(*it);
				if (++it == values.end())
					break;
				oss << value_;
			}
			return String(oss.str());
		}
	private:
		string value_;
	};
	String operator+(String left, const char* right);
	String operator+(String left, const string& right);
	String operator+(String left, const String& right);
	String operator+(const char* left, const String& right);
	String operator+(const string& left, const String& right);
} // namespace moss

template<>
struct std::hash<moss::String> {
	std::size_t operator()(const moss::String& s) const noexcept {
		return std::hash<std::string>{}(s.str());
	}
};

