#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdint>

namespace utils {

#ifdef linux

bool _kbhit();
char _getch();

#ifndef __max
#define __max(a,b) std::max(a,b)
#endif // !__max
#ifndef __min
#define __min(a,b) std::min(a,b)
#endif // !__min

#elif defined(_WIN32) || defined(WIN32)

#ifndef __max
#define __max(a,b) (((a) > (b)) ? (a) : (b))
#endif // !__max
#ifndef __min
#define __min(a,b) (((a) < (b)) ? (a) : (b))
#endif // !__min

#endif // !linux

	class StringUtils {
	public:
		static std::string ltrim(std::string s);
		static std::string rtrim(std::string s);
		static std::string trim(std::string s);
		static std::string replace(std::string str, const std::string& from, const std::string& to);
		static void replace_inline(std::string& str, const std::string& from, const std::string& to);
		static void replace_first(std::string& str, const std::string& from, const std::string& to);
		static std::string tolower(std::string str);
		static bool contains(const std::string& string, const std::string& cont);

		static std::string join(const std::vector<std::string>& strings, const char* const delim);
		static std::vector<std::string> split(const std::string& str, char delimiter);
		static std::vector<std::string> split(std::string s, const std::string& delimiter);

		static intmax_t hashcode(const std::string& str);

	};

	class CollectionUtils {
	public:
		template<typename Container, typename T>
		static bool contains(const Container& c, const T& value) {
			return std::find(c.begin(), c.end(), value) != c.end();
		}
	};

	class PathUtils {
	public:
		static std::string get_current_path();
		static std::string normalize_path_sep(const std::string& path);
	};

	class FlexaUUID {
	public:
		static std::string generate();
	};

}

#endif // !UTILS_HPP
