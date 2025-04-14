#include "flx_interpreter.hpp"

#include <filesystem>

#include "lexer.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include "dependency_resolver.hpp"
#include "interpreter.hpp"

using namespace interpreter;
using namespace core;
using namespace core::parser;
using namespace core::analysis;
using namespace core::runtime;

FlexaInterpreter::FlexaInterpreter(const FlexaCliArgs& args)
	: project_root(utils::PathUtils::normalize_path_sep(args.workspace_path)),
	libs_root(utils::PathUtils::normalize_path_sep((args.libs_path.empty() ? utils::PathUtils::get_current_path() : args.libs_path) + "\\libs")),
	args(args) {}

int FlexaInterpreter::execute() {
	if (!args.main_file.empty() || args.source_files.size() > 0) {
		return interpreter();
	}

	return 0;
}

FlexaSource FlexaInterpreter::load_program(const std::string& source) {
	FlexaSource source_program;

	auto current_file_path = std::string{ std::filesystem::path::preferred_separator } + utils::PathUtils::normalize_path_sep(source);
	std::string current_full_path = "";

	if (std::filesystem::exists(project_root + current_file_path)) {
		current_full_path = project_root + current_file_path;
	}
	else if (std::filesystem::exists(libs_root + current_file_path)) {
		current_full_path = libs_root + current_file_path;
	}
	else {
		throw std::runtime_error("file not found: '" + current_file_path + "'");
	}

	source_program = FlexaSource{ FlxUtils::get_lib_name(source), FlxUtils::load_source(current_full_path) };

	return source_program;
}

std::vector<FlexaSource> FlexaInterpreter::load_programs(const std::vector<std::string>& source_files) {
	std::vector<FlexaSource> source_programs;

	for (const auto& source : source_files) {
		source_programs.push_back(load_program(source));
	}

	return source_programs;
}

void FlexaInterpreter::parse_programs(const std::vector<FlexaSource>& source_programs, std::shared_ptr<ASTProgramNode>* main_program,
	std::map<std::string, std::shared_ptr<ASTProgramNode>>* programs) {

	for (const auto& source : source_programs) {
		Lexer lexer(source.name, source.source);
		parser::Parser parser(source.name , &lexer);

		std::shared_ptr<ASTProgramNode> program = parser.parse_program();

		if (!program) {
			std::cerr << "Failed to parse program: " << source.name << std::endl;
			continue;
		}

		if (!*main_program) {
			*main_program = program;
		}

		(*programs)[program->name] = program;
	}
}

int FlexaInterpreter::interpreter() {
	FlexaSource main_program_src;
	std::vector<FlexaSource> source_programs;
	try {
		main_program_src = load_program(args.main_file);
		source_programs = load_programs(args.source_files);
		source_programs.emplace(source_programs.begin(), main_program_src);
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	std::shared_ptr<Scope> semantic_global_scope = std::make_shared<Scope>(nullptr);
	std::shared_ptr<Scope> interpreter_global_scope = std::make_shared<Scope>(nullptr);

	try {
		std::shared_ptr<ASTProgramNode> main_program = nullptr;
		std::map<std::string, std::shared_ptr<ASTProgramNode>> programs;
		parse_programs(source_programs, &main_program, &programs);
		size_t libs_size = 0;
		do {
			DependencyResolver libfinder(main_program, programs);
			libfinder.start();

			libs_size = libfinder.lib_names.size();

			if (libs_size > 0) {
				auto cplib_programs = load_programs(libfinder.lib_names);
				parse_programs(cplib_programs, &main_program, &programs);
			}
		} while (libs_size > 0);

		semantic_global_scope->owner = main_program;
		interpreter_global_scope->owner = main_program;

		SemanticAnalyser semantic_analyser(semantic_global_scope, main_program, programs, args.program_args);
		semantic_analyser.start();

		intmax_t result = 0;

		if (args.engine == "ast") {
			Interpreter interpreter(interpreter_global_scope, main_program, programs, args.program_args);
			interpreter.start();
			result = interpreter.current_expression_value->get_i();
		}
		else {
			throw std::runtime_error("not implemented yet");
		}

		return result;
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
