#include "time_helper.h"

#include <sstream>
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#define LocalTime(t, r) localtime_s(t, r)
#else // OS_WIN
#include <sys/time.h>
#	define LocalTime(t, r) localtime_r(r, t)
#endif


namespace moss {
	Time::Time()
		: stp_(system_clock::now()),
		ctp_(steady_clock::now()) {
	}

	Time::Time(time_t t)
		: stp_(system_clock::from_time_t(t)),
		ctp_(steady_clock::now()) {
	}

	time_t Time::Timestamp() const {
		return system_clock::to_time_t(stp_);
	}

	int64_t Time::Timestamp(Time::Precision precision) const {
		switch (precision) {
		case Precision::MilliSecond:
			return duration_cast<milliseconds>(stp_.time_since_epoch()).count();
		case Precision::MicroSecond:
			return duration_cast<microseconds>(stp_.time_since_epoch()).count();
		default:
			return (int64_t)Timestamp();
		}
	}

	struct tm* Time::CurrentTimestamp(struct tm* t) const {
		time_t timestamp = Timestamp();
		LocalTime(t, &timestamp);
		return t;
	}

	string Time::Format(const char* fmt) const {
		struct tm t;
		const int size = 32;
		char prefix[size] = { 0 };
		strftime(prefix, size, fmt, CurrentTimestamp(&t));
		return string(prefix);
	}

	string Time::Format() const {
		const int size = 32;
		char buffer[size] = { 0 };
		snprintf(buffer, sizeof(buffer), "%s.%03d", Format("%Y-%m-%d %H:%M:%S").c_str(), int(Timestamp(Precision::MilliSecond) % 1000));
		return string(buffer);
	}

	time_t Time::Elapse(Time::Precision precision/* = Time::Precision::MilliSecond*/) const {
		auto now = steady_clock::now();
		switch (precision) {
		case Precision::MilliSecond:
			return duration_cast<milliseconds>(now - ctp_).count();
		case Precision::MicroSecond:
			return duration_cast<microseconds>(now - ctp_).count();
		default:
			return duration_cast<seconds>(now - ctp_).count();
		}
	}

	string Time::Print() const {
		std::ostringstream oss;
		oss << time(nullptr);
		return oss.str();
	}
	int Time::Year() const {
		struct tm t;
		return CurrentTimestamp(&t)->tm_year + 1900;
	}

	int Time::Month() const {
		struct tm t;
		return CurrentTimestamp(&t)->tm_mon - 1;
	}

	int Time::MonthDay() const {
		struct tm t;
		return CurrentTimestamp(&t)->tm_mday;
	}

	int Time::WeekDay() const {
		struct tm t;
		return CurrentTimestamp(&t)->tm_wday;
	}

	int Time::YearDay() const {
		struct tm t;
		return CurrentTimestamp(&t)->tm_yday;
	}

	int Time::Hour() const {
		struct tm t;
		return CurrentTimestamp(&t)->tm_hour;
	}

	int Time::Minute() const {
		struct tm t;
		return CurrentTimestamp(&t)->tm_min;
	}

	int Time::Second() const {
		struct tm t;
		return CurrentTimestamp(&t)->tm_sec;
	}

	Time Time::Truncate(Time::Unit unit) const {
		struct tm t;
		CurrentTimestamp(&t);
		switch (unit) {
		case Unit::Year:
			t.tm_mon = 0;
		case Unit::Month:
			t.tm_mday = 1;
		case Unit::Day:
			t.tm_hour = 0;
		case Unit::Hour:
			t.tm_min = 0;
		case Unit::Minute:
			t.tm_sec = 0;
		case Unit::Second:
			return Time(mktime(&t));
		case Unit::Week:
			t.tm_sec = 0;
			t.tm_min = 0;
			t.tm_hour = 0;
			return Time(mktime(&t) - (time_t)(t.tm_wday == 0 ? 6 : (t.tm_wday - 1)) * 24 * 3600);
		default:
			return *this;
		}
	}

	Time Time::Offset(int duration, Time::Unit unit) const {
		const int mdays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
		struct tm t;
		CurrentTimestamp(&t);
		switch (unit) {
		case Unit::Year:
			t.tm_year += duration;
			return Time(mktime(&t));
		case Unit::Month: {
			if (duration > 12 || duration < 12) {
				t.tm_year += duration / 12;
			}
			t.tm_mon += duration % 12;
			if (t.tm_mon < 0 || t.tm_mon > 11) {
				t.tm_year += (duration < 0) ? -1 : 1;
				t.tm_mon += (duration < 0) ? 12 : -12;
			}
			int mday = mdays[t.tm_mon % 12];
			if (((t.tm_year % 4 == 0 && t.tm_year % 100 != 0) || (t.tm_year % 400 == 0)) && t.tm_mon == 1) {
				mday += 1;
			}
			if (t.tm_mday > mday) {
				t.tm_mday = mday;
			}
			return Time(mktime(&t));
		}
		case Unit::Day:
			return Time(Timestamp() + (time_t)duration * 24 * 3600);
		case Unit::Hour:
			return Time(Timestamp() + (time_t)duration * 3600);
		case Unit::Minute:
			return Time(Timestamp() + (time_t)duration * 60);
		case Unit::Second:
			return Time(Timestamp() + (time_t)duration);
		case Unit::Week:
			return Time(Timestamp() + (time_t)duration * 7 * 24 * 3600);
		default:
			return *this;
		}
	}

	Time Time::Date() const {
		struct tm t;
		CurrentTimestamp(&t);
		t.tm_hour = 0;
		t.tm_min = 0;
		t.tm_sec = 0;
		return Time(mktime(&t));
	}

	Time Time::operator+(time_t count) const {
		Time t;
		t.stp_ += seconds(count);
		t.ctp_ += seconds(count);
		return t;
	}

	Time Time::operator-(time_t count) const {
		Time t;
		t.stp_ -= seconds(count);
		t.ctp_ -= seconds(count);
		return t;
	}

	Time& Time::operator+=(time_t count) {
		stp_ += seconds(count);
		ctp_ += seconds(count);
		return *this;
	}

	Time& Time::operator-=(time_t count) {
		stp_ -= seconds(count);
		ctp_ -= seconds(count);
		return *this;
	}

	TimeUnit::TimeUnit(int size, const string& unit)
		: size_(size), unit_(unit.c_str()) {
	}

	TimeUnit::~TimeUnit() {
	}

	string TimeUnit::Label() const {
		if (size_ > 0) {
			return "next" + unit_;
		} else if (size_ < 0) {
			return "last" + unit_;
		} else {
			return "current" + unit_;
		}
	}


	Day::Day(int size)
		: TimeUnit(size, "Day") {
	}

	std::tuple<int, int> Day::Value(int now) const {
		const int duration = 24 * 3600;
		time_t timestamp = now;
		struct tm t;
		LocalTime(&t, &timestamp);
		t.tm_min = 0;
		t.tm_sec = 0;
		t.tm_hour = 0;
		time_t start = mktime(&t) + (time_t)size_ * duration;
		time_t end = start + duration;
		return std::make_tuple((int)start, (int)end);
	}

	string Day::Label() const {
		if (size_ > 0) {
			return "tomorrow";
		} else if (size_ < 0) {
			return "yesterday";
		} else {
			return "today";
		}
	}


	Week::Week(int size)
		: TimeUnit(size, "Week") {
	}

	std::tuple<int, int> Week::Value(int now) const {
		const int duration = 7 * 24 * 3600;
		time_t timestamp = now;
		struct tm t;
		LocalTime(&t, &timestamp);
		t.tm_min = 0;
		t.tm_sec = 0;
		t.tm_hour = 0;
		time_t start = mktime(&t) - (((time_t)t.tm_wday - 1) * 24 * 3600) + (time_t)size_ * duration;
		time_t end = start + duration;
		return std::make_tuple((int)start, (int)end);
	}


	Month::Month(int size)
		: TimeUnit(size, "Month") {
	}

	std::tuple<int, int> Month::Value(int now) const {
		time_t timestamp = now;
		struct tm t;
		LocalTime(&t, &timestamp);
		t.tm_min = 0;
		t.tm_sec = 0;
		t.tm_hour = 0;
		t.tm_mday = 1;
		if (size_ < 0) {
			int ny = (-size_ / 12) * 12;
			int nm = -size_ % 12;
			t.tm_mon -= nm;
			t.tm_year -= ny;
			if (t.tm_mon < 0) {
				t.tm_mon += 12;
				t.tm_year -= 1;
			}
		} else {
			int ny = (size_ / 12) * 12;
			int nm = size_ % 12;
			t.tm_mon += nm;
			t.tm_year += ny;
			if (t.tm_mon > 11) {
				t.tm_mon -= 12;
				t.tm_year += 1;
			}
		}
		time_t start = mktime(&t);
		t.tm_mon += (size_ < 0) ? -size_ : size_;
		if (size_ == 0) {
			t.tm_mon += 1;
		}
		if (t.tm_mon > 11) {
			t.tm_year += t.tm_mon / 12;
			t.tm_mon = t.tm_mon % 12;
		}
		time_t end = mktime(&t);
		return std::make_tuple((int)start, (int)end);
	}


	Year::Year(int size)
		: TimeUnit(size, "Year") {
	}

	std::tuple<int, int> Year::Value(int now) const {
		time_t timestamp = now;
		struct tm t;
		LocalTime(&t, &timestamp);
		t.tm_min = 0;
		t.tm_sec = 0;
		t.tm_hour = 0;
		t.tm_mday = 1;
		t.tm_mon = 0;
		t.tm_year += size_;
		time_t start = mktime(&t);
		t.tm_year += (size_ < 0) ? -size_ : size_;
		t.tm_year += 1;
		time_t end = mktime(&t);
		return std::make_tuple((int)start, (int)end);
	}
} // namespace moss


