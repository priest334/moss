#pragma once

#include <ctime>
#include <chrono>
#include <string>
#include <tuple>


struct timeval;
using std::string;
using std::tuple;
using namespace std::chrono;
namespace moss {
	class Time {
	public:
		enum class Unit {
			Second = 0,
			Minute,
			Hour,
			Day,
			Week,
			Month,
			Year,
			MilliSecond,
			MicroSecond
		};
		enum class Precision {
			Second,
			MilliSecond,
			MicroSecond
		};
		Time();
		Time(time_t t);
		time_t Timestamp() const;
		int64_t Timestamp(Time::Precision precision) const;
		struct tm* CurrentTimestamp(struct tm* t) const;
		string Format(const char* fmt) const;
		string Format() const;
		time_t Elapse(Time::Precision precision = Time::Precision::MilliSecond) const;
		string Print() const;
		int Year() const;
		int Month() const;
		int MonthDay() const;
		int WeekDay() const;
		int YearDay() const;
		int Hour() const;
		int Minute() const;
		int Second() const;
		Time Truncate(Time::Unit unit) const;
		Time Offset(int duration, Time::Unit unit) const;
		Time Date() const;
		Time operator+(time_t count) const;
		Time operator-(time_t count) const;
		Time& operator+=(time_t count);
		Time& operator-=(time_t count);
	private:
		system_clock::time_point stp_;
		steady_clock::time_point ctp_;
	};

	class TimeUnit {
	public:
		TimeUnit(int size, const string& unit);
		virtual ~TimeUnit();
		virtual std::tuple<int, int> Value(int now) const = 0;
		virtual string Label() const;
	protected:
		int size_;
		string unit_;
	};

	class Day : public TimeUnit {
	public:
		Day(int size);
		std::tuple<int, int> Value(int now) const override;
		string Label() const override;
	};

	class Week : public TimeUnit {
	public:
		Week(int size);
		std::tuple<int, int> Value(int now) const override;
	};

	class Month : public TimeUnit {
	public:
		Month(int size);
		std::tuple<int, int> Value(int now) const override;
	};

	class Year : public TimeUnit {
	public:
		Year(int size);
		std::tuple<int, int> Value(int now) const override;
	};
} // namespace moss

