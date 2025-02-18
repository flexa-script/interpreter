#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <ctime>

#include "logging.hpp"

#pragma warning(disable : 4996)

using namespace utils;

Logger::Logger(LogLevel level) : level(level), path("") {}

Logger::Logger(LogLevel level, std::string path) : level(level), path(path) {}

void Logger::debug(std::string msg) {
	if (is_enabled_for(LogLevel::DEBUG)) {
		log(LogLevel::DEBUG, msg);
	}
}

void Logger::info(std::string msg) {
	if (is_enabled_for(LogLevel::INFO)) {
		log(LogLevel::INFO, msg);
	}
}

void Logger::warning(std::string msg) {
	if (is_enabled_for(LogLevel::WARNING)) {
		log(LogLevel::WARNING, msg);
	}
}

void Logger::error(std::string msg) {
	if (is_enabled_for(LogLevel::ERROR)) {
		log(LogLevel::ERROR, msg);
	}
}

void Logger::critical(std::string msg) {
	if (is_enabled_for(LogLevel::CRITICAL)) {
		log(LogLevel::CRITICAL, msg);
	}
}

LogLevel Logger::name_to_level(std::string name) {
	return LEVEL_LOG.at(name);
}

std::string Logger::level_to_name(LogLevel level) {
	return LEVEL_NAME[level];
}

bool Logger::is_enabled_for(LogLevel level) {
	return level >= this->level;
}

void Logger::log(LogLevel level, std::string msg) {
	std::stringstream ss;
	std::string s;
	char buffer[32];
	auto now = std::chrono::system_clock::now();
	auto time = std::chrono::system_clock::to_time_t(now);
	std::tm* ptm = std::localtime(&time);
	std::strftime(buffer, 32, "%Y-%m-%d %H:%M:%S", ptm);
	ss << "[" << buffer << "] " << level_to_name(level) << ": " << msg << std::endl;
	s = ss.str();
	write(s);
	std::cout << s;
}

void Logger::write(std::string log_message){
	if (path.empty()) {
		return;
	}
	std::ofstream of;
	of.open(path, std::ios_base::app);
	if (of.is_open()) {
		of << log_message;
	}
	of.close();
}
