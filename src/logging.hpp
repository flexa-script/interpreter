#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <string>
#include <map>

namespace utils {

	enum LogLevel {
		NOTSET,
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		CRITICAL
	};

	const std::map<std::string, LogLevel> LEVEL_LOG = {
		{"NOTSET", NOTSET},
		{"DEBUG", DEBUG},
		{"INFO", INFO},
		{"WARNING", WARNING},
		{"ERROR", ERROR},
		{"CRITICAL", CRITICAL}
	};

	const std::string LEVEL_NAME[] = {
		"NOTSET"
		"DEBUG",
		"INFO",
		"WARNING",
		"ERROR",
		"CRITICAL",
	};

	class Logger {
	private:
		LogLevel level;
		std::string path;

	public:
		Logger(LogLevel);
		Logger(LogLevel, std::string);
		Logger() = default;

		void debug(std::string);
		void info(std::string);
		void warning(std::string);
		void error(std::string);
		void critical(std::string);

		LogLevel name_to_level(std::string);
		std::string level_to_name(LogLevel);

	private:
		bool is_enabled_for(LogLevel);

		void log(LogLevel, std::string);
		void write(std::string);
	};
}

#endif // !LOGGING_HPP
