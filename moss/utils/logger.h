#pragma once

#include <cstdarg>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>


using std::shared_ptr;
using std::string;
using std::mutex;
namespace moss {
	enum class Severity: unsigned int {
		Debug = 0,
		Info,
		Warning,
		Error,
		Fatal,
		MaxValue
	};

	class LogWriter {
	public:
		virtual ~LogWriter();
		virtual void Write(const string& message) = 0;
	};

	class Console : public LogWriter {
	public:
		Console();
		void Write(const string& message) override;
	private:
		mutex mutex_;
	};

	class SingleFile : public LogWriter {
	public:
		SingleFile(const string& filename);
		void Write(const string& message) override;
	private:
		mutex mutex_;
		std::ofstream ofs_;
	};

	class Logger;
	class Message : public std::ostringstream {
	public:
		Message(Severity severity, shared_ptr<Logger> logger);
		virtual ~Message();
		Message& Format(const char* fmt, ...);
	private:
		Severity severity_;
		shared_ptr<Logger> logger_;
	};

	class Logger {
		typedef Logger& (Logger::*WriteMethod)(const Message&);
	public:
		Logger(Severity severity = Severity::Debug);
		~Logger();
		Logger& SetLevel(Severity severity);
		Logger& SetWriter(shared_ptr<LogWriter> writer);

		Logger& Debug(const Message& message);
		Logger& Info(const Message& message);
		Logger& Warning(const Message& message);
		Logger& Error(const Message& message);
		Logger& Fatal(const Message& message);
		Logger& Write(Severity severity, const Message& message);
	private:
		Severity severity_;
		shared_ptr<LogWriter> writer_;
		Logger::WriteMethod methods_[(unsigned int)Severity::MaxValue];
	};

	namespace logger {
		shared_ptr<Logger> GetDefault();
		void SetWriter(shared_ptr<LogWriter> writer);

		class Debug 
			: public Message {
		public:
			Debug();
			Debug(shared_ptr<Logger> logger);
			Debug(const char* file, int line);
			Debug(shared_ptr<Logger> logger, const char* file, int line);
		};

		class Info 
			: public Message {
		public:
			Info();
			Info(shared_ptr<Logger> logger);
			Info(const char* file, int line);
			Info(shared_ptr<Logger> logger, const char* file, int line);
		};

		class Warning 
			: public Message {
		public:
			Warning();
			Warning(shared_ptr<Logger> logger);
			Warning(const char* file, int line);
			Warning(shared_ptr<Logger> logger, const char* file, int line);
		};

		class Error 
			: public Message {
		public:
			Error();
			Error(shared_ptr<Logger> logger);
			Error(const char* file, int line);
			Error(shared_ptr<Logger> logger, const char* file, int line);
		};

		class Fatal 
			: public Message {
		public:
			Fatal();
			Fatal(shared_ptr<Logger> logger);
			Fatal(const char* file, int line);
			Fatal(shared_ptr<Logger> logger, const char* file, int line);
		};
	} // namespace logger
} // namespace moss

