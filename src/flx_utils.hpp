#ifndef BSLUTILS_HPP
#define BSLUTILS_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <iomanip>
#include <vector>

struct FlexaSource {
	std::string name;
	std::string source;
};

extern std::string load_source(const std::string& path);

extern std::string get_lib_name(const std::string& libpath);

extern std::string get_prog_name(const std::string& progpath);

struct FlexaCliArgs {
	bool debug = false;
	std::string engine;
	std::string libs_path;
	std::string workspace_path;
	std::string main_file;
	std::vector<std::string> source_files;
	std::vector<std::string> program_args;
};

extern void throw_if_not_parameter(int argc, size_t i, std::string parameter);

extern FlexaCliArgs parse_args(int argc, const char* argv[]);

#endif // !BSLUTILS_HPP
