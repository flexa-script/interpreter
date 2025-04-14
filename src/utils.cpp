#include "utils.hpp"

#include <filesystem>
#include <random>
#include <iterator>

using namespace utils;

#ifdef linux

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <limits.h>

bool utils::_kbhit() {
    termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ret > 0;
}

char utils::_getch() {
    termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}

#elif defined(_WIN32) || defined(WIN32)

#include <Windows.h>

#endif // linux

// StringUtils

std::string StringUtils::ltrim(std::string s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
	return s;
}

std::string StringUtils::rtrim(std::string s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
	return s;
}

std::string StringUtils::trim(std::string s) {
	return ltrim(rtrim(s));
}

std::string StringUtils::replace(std::string str, const std::string& from, const std::string& to) {
	size_t pos = 0;
	while ((pos = str.find(from, pos)) != std::string::npos) {
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
	return str;
}

void StringUtils::replace_inline(std::string& str, const std::string& from, const std::string& to) {
	size_t pos = 0;
	while ((pos = str.find(from, pos)) != std::string::npos) {
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}

void StringUtils::replace_first(std::string& str, const std::string& from, const std::string& to) {
	std::size_t pos = str.find(from);
	if (pos == std::string::npos) return;
	str.replace(pos, from.length(), to);
}

std::string StringUtils::tolower(std::string str) {
	std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
	return str;
}

bool StringUtils::contains(const std::string& string, const std::string& cont) {
	return string.find(cont) != std::string::npos;
}

std::string StringUtils::join(const std::vector<std::string>& strings, const char* const delim) {
	std::string string;
	std::ostringstream imploded;
	std::copy(strings.begin(), strings.end(), std::ostream_iterator<std::string>(imploded, delim));
	string = imploded.str();
	if (!string.empty()) string.pop_back();
	return string;
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
	std::vector<std::string> result;
	std::istringstream iss(str);
	std::string item;
	while (std::getline(iss, item, delimiter)) {
		result.push_back(item);
	}
	return result;
}

std::vector<std::string> StringUtils::split(std::string s, const std::string& delimiter) {
	std::vector<std::string> tokens;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		tokens.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	tokens.push_back(s);

	return tokens;
}

intmax_t StringUtils::hashcode(const std::string& str) {
	intmax_t h = 0;
	for (size_t i = 0; i < str.size(); ++i) {
		h = h * 31 + static_cast<intmax_t>(str[i]);
	}
	return h;
}

// CollectionUtils

// PathUtils

std::string PathUtils::get_current_path() {

#ifdef linux

	char result[PATH_MAX];
	ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);

    if (count == -1) {
        throw std::runtime_error("there was not possible to obtain executable path");
    }

    std::filesystem::path path(std::string(result, count));

#elif defined(_WIN32) || defined(WIN32)

	HMODULE this_process_handle = GetModuleHandle(NULL);
	wchar_t this_process_path[MAX_PATH];

	GetModuleFileNameW(NULL, this_process_path, sizeof(this_process_path));

    std::filesystem::path path(this_process_path);

#endif // linux

	return normalize_path_sep(path.remove_filename().generic_string());
}

std::string PathUtils::normalize_path_sep(const std::string& path) {
	std::string sep(std::string{ std::filesystem::path::preferred_separator });
	auto ret_str = StringUtils::replace(path, "/", sep);
	ret_str = StringUtils::replace(ret_str, "\\", sep);
	ret_str = StringUtils::replace(ret_str, sep + sep, sep);

	return ret_str;
}

// FlexaUUID

std::string utils::FlexaUUID::generate() {
	static std::random_device dev;
	static std::mt19937 rng(dev());

	std::uniform_int_distribution<int> dist(0, 15);

	const char* v = "0123456789abcdef";
	const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

	std::string res;
	for (int i = 0; i < 16; i++) {
		if (dash[i]) res += "-";
		res += v[dist(rng)];
		res += v[dist(rng)];
	}

	return res;
}
