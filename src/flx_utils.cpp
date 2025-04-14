#include "flx_utils.hpp"

#include <filesystem>

#include "utils.hpp"

using namespace interpreter;
using namespace utils;

std::string FlxUtils::load_source(const std::string& path) {
	std::string source;

	std::ifstream file;
	file.open(path);

	if (!file) {
		std::cout << "Could not load file from \"" << path << "\"." << std::endl;
	}
	else {
		std::string line;
		while (std::getline(file, line)) {
			source.append(line + "\n");
		}
	}

	if ((unsigned char)source[0] == 0xEF &&
		(unsigned char)source[1] == 0xBB &&
		(unsigned char)source[2] == 0xBF) {
		source = source.substr(3, source.size());
	}

	return source;
}

std::string FlxUtils::get_lib_name(const std::string& libpath) {
	std::string file_name = libpath;
	std::string lib_name = file_name.substr(0, file_name.length() - 4);
	std::replace(lib_name.begin(), lib_name.end(), (char)std::filesystem::path::preferred_separator, '.');
	return lib_name;
}

std::string FlxUtils::get_prog_name(const std::string& progpath) {
	auto norm_path = utils::PathUtils::normalize_path_sep(progpath);
	auto index = norm_path.rfind(std::filesystem::path::preferred_separator) + 1;
	return FlxUtils::get_lib_name(progpath.substr(index, progpath.size()));
}

FlexaCliArgs::FlexaCliArgs(int argc, const char* argv[]) {
	for (int i = 0; i < argc; ++i) {
		args.push_back(argv[i]);
	}
	parse_args();
}

void FlexaCliArgs::parse_args() {
	auto args_size = args.size();
	engine = "ast";

	size_t i = 0;

	program_args.push_back(args[i]);

	// parse optional parameters
	while (++i < args_size) {
		std::string arg = args[i];
		if (arg == "-d" || arg == "--debug") {
			debug = true;

			continue;
		}
		if (arg == "-e" || arg == "--engine") {
			++i;
			throw_if_not_parameter(args_size, i, arg);
			std::string p = args[i];
			if (p != "ast" && p != "vm") {
				throw std::runtime_error("invalid " + arg + " parameter value: '" + p + "'");
			}
			engine = args[i];
			continue;
		}
		if (arg == "-w" || arg == "--workspace") {
			++i;
			throw_if_not_parameter(args_size, i, arg);
			workspace_path = args[i];
			continue;
		}
		if (arg == "-m" || arg == "--main") {
			++i;
			throw_if_not_parameter(args_size, i, arg);
			main_file = args[i];
			continue;
		}
		if (arg == "-s" || arg == "--source") {
			++i;
			throw_if_not_parameter(args_size, i, arg);
			source_files.push_back(args[i]);
			continue;
		}
		if (arg == "-l" || arg == "--libs") {
			++i;
			throw_if_not_parameter(args_size, i, arg);
			libs_path = args[i];
			continue;
		}
		--i;
		// no more optional parameters
		break;
	}

	// check if the first
	auto ni = i + 1;
	if (ni < args_size && std::filesystem::is_regular_file(args[ni]) && workspace_path.empty()) {
		std::filesystem::path full_path(args[ni]);
		workspace_path = full_path.parent_path().string();
		main_file = full_path.filename().string();
	}
	else {
		std::filesystem::path w_path(workspace_path);
		std::filesystem::path m_path(main_file);
		program_args.push_back((w_path / m_path).string());
	}

	// parse flx arguments
	while (++i < args_size) {
		program_args.push_back(args[i]);
	}

}

void FlexaCliArgs::throw_if_not_parameter(int argc, size_t i, std::string parameter) {
	if (i >= argc) {
		throw std::runtime_error("expected value after " + parameter);
	}
}
