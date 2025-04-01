#ifndef FLX_UTILS_HPP
#define FLX_UTILS_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <iomanip>
#include <vector>

namespace interpreter {

	struct FlexaSource {
		std::string name;
		std::string source;
	};

	class FlxUtils {
	public:
		static std::string load_source(const std::string& path);
		static std::string get_lib_name(const std::string& libpath);
		static std::string get_prog_name(const std::string& progpath);

	};

	class FlexaCliArgs {
	private:
		std::vector<std::string> args;

	public:
		bool debug = false;
		std::string engine;
		std::string libs_path;
		std::string workspace_path;
		std::string main_file;
		std::vector<std::string> source_files;
		std::vector<std::string> program_args;

		FlexaCliArgs(int argc, const char* argv[]);

		void parse_args();

		void throw_if_not_parameter(int argc, size_t i, std::string parameter);

	};

}

#endif // !FLX_UTILS_HPP
