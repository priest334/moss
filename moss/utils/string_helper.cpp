#include "string_helper.h"

#include <algorithm>
#include <codecvt>
#include <random>
#include <regex>


#if defined(WIN32)
#define I64d "i64d"
#define StringPrintf _vsnprintf
//#pragma message("#pragma warning(disable: 4996) or define _CRT_SECURE_NO_WARNINGS")
#pragma warning(disable: 4996)
#define StrCmpI stricmp
#define StrCmpNI strnicmp
#else // !_WIN2
#define I64d "llu"
#define StringPrintf vsnprintf
#define StrCmpI strcasecmp
#define StrCmpNI strncasecmp
#endif


namespace moss {
	String String::Digits = "0123456789";
	String String::CapitalLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	String String::SmallLetters = "abcdefghijklmnopqrstuvwxyz";
	String String::Letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	String String::Normal = "01234567890ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	String String::Printable = " !\"#$%&'()*+,-./01234567890:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	String String::WhiteSpace = "\t\r\n ";

	String String::Random(size_t length, const String& chars) {
		String value(length);
		size_t count = chars.length();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distrib(0, int(count - 1));
		for (size_t i = 0; i < length; i++) {
			value[(int)i] = chars[distrib(gen)];
		}
		return value;
	}

	String String::Random(size_t length) {
		return String::Random(length, String::Normal);
	}

	String::String() {
	}

	String::String(size_t size, char ch/* = '\0'*/)
		: value_(size, ch) {
	}

	String::String(const char* str)
		: value_(str) {
	}

	String::String(const char* str, size_t length)
		: value_(str, length) {
	}

	String::String(const string& str)
		: value_(str.c_str(), str.length()) {
	}

	String::String(const String& str)
		: value_(str.value_.c_str(), str.value_.length()) {
	}

	String& String::Format(const char* fmt, ...) {
		size_t size = 0;
		va_list args;
		va_start(args, fmt);
		char* dest = nullptr;
		int length = StringPrintf(dest, size, fmt, args);
		va_end(args);
		if (length < 0) {
			return *this;
		}
		size =(size_t)length + 1;
		value_.resize(size);
		va_start(args, fmt);
		length = StringPrintf(&value_[0], size, fmt, args);
		va_end(args);
		if (length < 0) {
			return *this;
		}
		value_.resize(length);
		return *this;
	}

	String& String::Assign(const char* str) {
		if (str) {
			value_.assign(str);
		} else {
			value_.clear();
		}
		return *this;
	}

	String& String::Assign(const char* str, size_t length) {
		if (str) {
			value_.assign(str, length);
		} else {
			value_.clear();
		}
		return *this;
	}

	String& String::Assign(const string& str) {
		value_.assign(str);
		return *this;
	}

	String& String::Assign(const String& str) {
		value_.assign(str.value_);
		return *this;
	}

	String& String::Append(const char* str) {
		if (str) {
			value_.append(str);
		}
		return *this;
	}

	String& String::Append(const char* str, size_t length) {
		if (str) {
			value_.append(str, length);
		}
		return *this;
	}

	String& String::Append(const string& str) {
		value_.append(str);
		return *this;
	}

	String& String::Append(const String& str) {
		value_.append(str.value_);
		return *this;
	}

	String String::Center(size_t length, char fill/* = ' '*/) const {
		if (value_.empty()) {
			return String(string(length, fill));
		}
		size_t size = value_.length();
		size_t right = (length - size) / 2;
		size_t left = length - size - right;
		return String(string(left, fill) + value_ + string(right, fill));
	}

	String String::StripLeft(const String& chars) const {
		if (value_.empty() || chars.value_.empty())
			return String(value_);
		size_t fpos = value_.find_first_not_of(chars.value_);
		if (fpos == string::npos) {
			return String();
		}
		return String(value_.substr(fpos));
	}

	String String::StripLeft() const {
		return StripLeft("\r\n\t ");
	}

	String String::StripRight(const String& chars) const {
		if (value_.empty() || chars.value_.empty())
			return String(value_);
		size_t fpos = value_.find_last_not_of(chars.value_);
		if (fpos == string::npos) {
			return String();
		}
		return String(value_.substr(0, fpos + 1));
	}

	String String::StripRight() const {
		return StripRight("\r\n\t ");
	}

	String String::Strip(const String& chars) const {
		return StripLeft(chars).StripRight(chars);
	}

	String String::Strip() const {
		return Strip("\r\n\t ");
	}

	String String::SubStr(int start, int end/* = -1*/) const {
		int s = (start < 0) ? ((int)value_.length() + start) : start;
		int e = (end < 0) ? ((int)value_.length() + end) : end;
		if (s < e || s < 0) {
			return String();
		}
		return value_.substr(s, e < 0 ? -1 : e - s + 1);
	}

	String String::RemovePrefix(const String& str) const {
		if (str.value_.empty() || value_.empty())
			return String(value_);
		if (value_.length() >= str.value_.length() && 0 == value_.find(str.value_))
			return String(value_.substr(str.value_.length()));
		return String(value_);
	}

	String String::RemoveSuffix(const String& str) const {
		if (str.value_.empty() || value_.empty())
			return String(value_);
		size_t fpos = string::npos;
		if (value_.length() >= str.value_.length() && fpos == value_.rfind(str.value_))
			return String(value_.substr(0, fpos));
		return String(value_);
	}

	String String::Replace(const String& old_str, const String& new_str) const {
		if (old_str.value_.empty() || new_str.value_.empty())
			return String(value_);
		string new_value;
		size_t fpos = string::npos, offset = 0;
		size_t old_length = old_str.value_.length();
		fpos = value_.find(old_str.value_, offset);
		while (fpos != string::npos) {
			new_value += value_.substr(offset, fpos - offset);
			new_value += new_str.value_;
			offset = fpos + old_length;
			fpos = value_.find(old_str.value_, offset);
		} 
		new_value += value_.substr(offset);
		return String(new_value);
	}

	vector<string> String::split(const string& seperator, size_t maxsplit/* = -1*/) const {
		vector<string> values;
		if (seperator.empty() || value_.empty()) {
			values.push_back(value_);
			return values;
		}
		size_t fpos, offset = 0, seplen = seperator.length(), count = 0;
		fpos = value_.find(seperator, offset);
		while (fpos != string::npos) {
			string value = value_.substr(offset, fpos - offset);
			if (!value.empty()) {
				values.push_back(value);
				if (++count > maxsplit)
					break;
			}
			offset = fpos + seplen;
			fpos = value_.find(seperator, offset);
		}
		string value = value_.substr(offset);
		if (!value.empty())
			values.push_back(value_.substr(offset));
		return values;
	}

	vector<String> String::Split(const String& seperator, size_t maxsplit/* = -1*/) const {
		auto values = split(seperator, maxsplit);
		vector<String> values1;
		values1.insert(values1.begin(), values.begin(), values.end());
		return values1;
	}

	unordered_map<string, string> String::split(const string& seperator, const string& assign_key) const {
		unordered_map<string, string> values;
		for (auto& parameter : Split(seperator)) {
			auto keys = parameter.Split(assign_key, 1);
			if (keys.size() != 2)
				continue;
			values.insert(std::make_pair(keys[0].Strip().str(), keys[1].Strip().str()));
		}
		return values;
	}

	unordered_map<String, String> String::Split(const String& seperator, const String& assign_key) const {
		auto values = split(seperator.str(), assign_key.str());
		unordered_map<String, String> values1;
		for (auto& it : values) {
			values1.insert(std::make_pair(it.first, it.second));
		}
		return values1;
	}

	String String::CamelToUnderscore() const {
		string new_value;
		if (value_.empty())
			return String(new_value);
		bool skip = false;
		for (size_t i = 0; i < value_.length(); i++) {
			char ch = value_[i];
			if (ch == '\'' || ch == '"') {
				skip = !skip;
				new_value.append(1, ch);
				continue;
			}
			if (skip) {
				new_value.append(1, ch);
				continue;
			}
			if (std::isupper(ch) && !skip) {
				if (!new_value.empty()) {
					new_value.append(1, '_');
				}
				new_value.append(1, std::tolower(ch));
			} else {
				new_value.append(1, ch);
			}
		}
		return String(new_value);
	}

	String String::UnderscoreToCamel(bool little/* = true*/) const {
		string new_value;
		if (value_.empty())
			return String(new_value);
		bool skip = false;
		bool upper = !little;
		for (size_t i = 0; i < value_.length(); i++) {
			char ch = value_[i];
			if (ch == '\'' || ch == '"') {
				skip = !skip;
				new_value.append(1, ch);
				continue;
			}
			if (skip) {
				new_value.append(1, ch);
				continue;
			}
			if (ch == '_' && i != 0) {
				upper = true;
				continue;
			}
			if (upper) {
				if (std::islower(ch)) {
					new_value.append(1, std::toupper(ch));
				} else {
					new_value.append(1, '_');
					new_value.append(1, ch);
				}
			} else {
				new_value.append(1, ch);
			}
			upper = false;
		}
		return String(new_value);
	}

	String String::Translate(const unordered_map<char, char>& table) const {
		string new_value;
		if (new_value.empty())
			return String(value_);
		for (size_t i = 0; i < value_.length(); i++) {
			char ch = value_[i];
			auto it = table.find(ch);
			if (it != table.end()) {
				new_value.append(1, it->second);
			} else {
				new_value.append(1, ch);
			}
		}
		return String(new_value);
	}

	String String::ToUpper() const {
		string new_value;
		for (size_t i = 0; i < value_.length(); i++) {
			new_value.append(1, std::toupper(value_[i]));
		}
		return String(new_value);
	}

	String String::ToLower() const {
		string new_value;
		for (size_t i = 0; i < value_.length(); i++) {
			new_value.append(1, std::tolower(value_[i]));
		}
		return String(new_value);
	}

	//String String::Utf8ToGbk() const {
	//	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;
	//	std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> gbk(new std::codecvt_byname<wchar_t, char, std::mbstate_t>("chs"));
	//	std::wstring unicode = utf8.from_bytes(value_);
	//	return gbk.to_bytes(unicode);
	//}

	//String String::GbkToUtf8() const {
	//	std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8;
	//	std::wstring_convert<std::codecvt_byname<wchar_t, char, std::mbstate_t>> gbk(new std::codecvt_byname<wchar_t, char, std::mbstate_t>("chs"));
	//	std::wstring unicode = gbk.from_bytes(value_);
	//	return utf8.to_bytes(unicode);
	//}

	bool String::IsDigit() const {
		for (size_t i = 0; i < value_.length(); i++) {
			char ch = value_[i];
			if (ch < '0' || ch > '9') {
				return false;
			}
		}
		return true;
	}

	bool String::IsNumeric() const {
		std::cmatch mr;
		std::regex pattern("\\d+(\\.\\d+)?");
		return std::regex_match(value_.c_str(), mr, pattern);;
	}

	bool String::IsHex() const {
		for (size_t i = 0; i < value_.length(); i++) {
			char ch = value_[i];
			if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F'))) {
				return false;
			}
		}
		return true;
	}

	bool String::StartsWith(const String& prefix) const {
		if (value_.empty() || prefix.value_.empty())
			return false;
		return 0 == value_.find(prefix.value_);
	}

	bool String::EndsWith(const String& suffix) const {
		if (value_.empty() || suffix.value_.empty())
			return false;
		size_t fpos = value_.length() - suffix.value_.length();
		return fpos != string::npos && fpos == value_.rfind(suffix.value_);
	}

	int String::Find(const String& sub) const {
		return (int)value_.find(sub.value_);
	}

	int String::ToInt() const {
		return std::strtol(value_.c_str(), nullptr, 10);
	}

	int64_t String::ToInt64() const {
		return (int64_t)std::strtoll(value_.c_str(), nullptr, 10);
	}

	float String::ToFloat() const {
		return (float)std::strtof(value_.c_str(), nullptr);
	}

	double String::ToDouble() const {
		return (double)std::strtod(value_.c_str(), nullptr);
	}

	String& String::resize(size_t new_size, const char fill/* = (char)0*/) {
		value_.resize(new_size, fill);
		return *this;
	}

	char& String::operator[](int index) {
		return value_[index];
	}

	char String::operator[](int index) const {
		int length = (int)value_.length();
		if (index >= 0 && index < length) {
			return value_[index];
		} else if (index < 0 && (index + length) >= 0) {
			return value_[index + length];
		}
		return '\0';
	}

	string String::str() const {
		return value_;
	}

	char const* String::c_str() const {
		return value_.c_str();
	}

	size_t String::length() const {
		return value_.length();
	}

	size_t String::size() const {
		return value_.size();
	}

	String::operator int() const {
		return std::strtol(value_.c_str(), nullptr, 10);
	}

	String::operator char const* () const {
		return value_.c_str();
	}

	String::operator string const& () const {
		return value_;
	}

	String& String::operator=(const char* str) {
		value_.assign(str);
		return *this;
	}

	String& String::operator=(const string& str) {
		value_.assign(str);
		return *this;
	}

	String& String::operator=(const String& str) {
		value_.assign(str.value_);
		return *this;
	}

	String& String::operator+=(const char* str) {
		value_.append(str);
		return *this;
	}

	String& String::operator+=(const string& str) {
		value_.append(str);
		return *this;
	}

	String& String::operator+=(const String& str) {
		value_.append(str.value_);
		return *this;
	}

	bool operator<(const char* left, const String& right) {
		return left < right.value_;
	}

	bool operator<(const String& left, const char* right) {
		return left.value_ < right;
	}

	bool operator<(const string& left, const String& right) {
		return left < right.value_;
	}

	bool operator<(const String& left, const string& right) {
		return left.value_ < right;
	}

	bool operator<(const String& left, const String& right) {
		return left.value_ < right.value_;
	}

	bool operator==(const char* left, const String& right) {
		return left == right.value_;
	}

	bool operator==(const String& left, const char* right) {
		return left.value_ == right;
	}

	bool operator==(const string& left, const String& right) {
		return left == right.value_;
	}

	bool operator==(const String& left, const string& right) {
		return left.value_ == right;
	}

	bool operator==(const String& left, const String& right) {
		return left.value_ == right.value_;
	}

	bool operator>(const char* left, const String& right) {
		return right < left;
	}

	bool operator>(const String& left, const char* right) {
		return right < left;
	}

	bool operator>(const string& left, const String& right) {
		return right < left;
	}

	bool operator>(const String& left, const string& right) {
		return right < left;
	}

	bool operator>(const String& left, const String& right) {
		return right < left;
	}

	bool operator!=(const char* left, const String& right) {
		return !(left == right);
	}

	bool operator!=(const String& left, const char* right) {
		return !(left == right);
	}

	bool operator!=(const string& left, const String& right) {
		return !(left == right);
	}

	bool operator!=(const String& left, const string& right) {
		return !(left == right);
	}

	bool operator!=(const String& left, const String& right) {
		return !(left == right);
	}

	bool operator<=(const char* left, const String& right) {
		return !(right > left);
	}

	bool operator<=(const String& left, const char* right) {
		return !(right > left);
	}

	bool operator<=(const string& left, const String& right) {
		return !(right > left);
	}

	bool operator<=(const String& left, const string& right) {
		return !(right > left);
	}

	bool operator<=(const String& left, const String& right) {
		return !(right > left);
	}

	bool operator>=(const char* left, const String& right) {
		return !(left < right);
	}

	bool operator>=(const String& left, const char* right) {
		return !(left < right);
	}

	bool operator>=(const string& left, const String& right) {
		return !(left < right);
	}

	bool operator>=(const String& left, const string& right) {
		return !(left < right);
	}

	bool operator>=(const String& left, const String& right) {
		return !(left < right);
	}

	String operator+(String left, const char* right) {
		left += right;
		return left;
	}

	String operator+(String left, const string& right) {
		left += right;
		return left;
	}

	String operator+(String left, const String& right) {
		left += right;
		return left;
	}

	String operator+(const char* left, const String& right) {
		String value = left;
		value += right;
		return value;
	}

	String operator+(const string& left, const String& right) {
		String value = left;
		value += right;
		return value;
	}

	std::ostream& operator<<(std::ostream& stream, const String& str) {
		stream << str.value_;
		return stream;
	}
} // namespace moss
