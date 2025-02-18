#include <filesystem>

#include "utils.hpp"
#include "flx_utils.hpp"

std::string load_source(const std::string& path) {
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

std::string get_lib_name(const std::string& libpath) {
	std::string file_name = libpath;
	std::string lib_name = file_name.substr(0, file_name.length() - 4);
	std::replace(lib_name.begin(), lib_name.end(), (char)std::filesystem::path::preferred_separator, '.');
	return lib_name;
}

std::string get_prog_name(const std::string& progpath) {
	auto norm_path = utils::PathUtils::normalize_path_sep(progpath);
	auto index = norm_path.rfind(std::filesystem::path::preferred_separator) + 1;
	return get_lib_name(progpath.substr(index, progpath.size()));
}

void throw_if_not_parameter(int argc, size_t i, std::string parameter) {
	if (i >= argc) {
		throw std::runtime_error("expected value after " + parameter);
	}
}

FlexaCliArgs parse_args(int argc, const char* argv[]) {
	FlexaCliArgs args;
	args.engine = "ast";

	size_t i = 0;

	args.program_args.push_back(argv[i]);

	// parse optional parameters
	while (++i < argc) {
		std::string arg = argv[i];
		if (arg == "-d" || arg == "--debug") {
			args.debug = true;

			continue;
		}
		if (arg == "-e" || arg == "--engine") {
			++i;
			throw_if_not_parameter(argc, i, arg);
			std::string p = argv[i];
			if (p != "ast" && p != "vm") {
				throw std::runtime_error("invalid " + arg + " parameter value: '" + p + "'");
			}
			args.engine = argv[i];
			continue;
		}
		if (arg == "-w" || arg == "--workspace") {
			++i;
			throw_if_not_parameter(argc, i, arg);
			args.workspace_path = argv[i];
			continue;
		}
		if (arg == "-m" || arg == "--main") {
			++i;
			throw_if_not_parameter(argc, i, arg);
			args.main_file = argv[i];
			continue;
		}
		if (arg == "-s" || arg == "--source") {
			++i;
			throw_if_not_parameter(argc, i, arg);
			args.source_files.push_back(argv[i]);
			continue;
		}
		--i;
		// no more optional parameters
		break;
	}

	// parse flx arguments
	while (++i < argc) {
		args.program_args.push_back(argv[i]);
	}

	return args;
}
