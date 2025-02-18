#include "flx_repl.hpp"
#include "utils.hpp"
#include "types.hpp"

const std::string FlexaRepl::NAME = "Flexa";
const std::string FlexaRepl::VER = "v0.0.1";
const std::string FlexaRepl::YEAR = "2025";

void FlexaRepl::remove_header(std::string& err) {
	size_t pos = err.rfind(':');
	if (pos != std::string::npos) {
		err = err.substr(pos + 1);
	}
}

std::string FlexaRepl::read(const std::string& msg) {
	std::string input_line;
	std::cout << msg;
	std::getline(std::cin, input_line);
	return input_line;
}

void FlexaRepl::count_scopes(const std::string& input_line, unsigned int& open_scopes) {
	open_scopes += std::count(input_line.begin(), input_line.end(), '{');
	open_scopes -= std::count(input_line.begin(), input_line.end(), '}');
}

int FlexaRepl::execute(const FlexaCliArgs& args) {
	std::cout << NAME << " " << VER << " [" << YEAR << "]\n";
	std::cout << "Type \"#help\" for more information.\n";

	std::shared_ptr<visitor::Scope> semantic_global_scope = std::make_shared<visitor::Scope>(nullptr);
	std::shared_ptr<visitor::Scope> interpreter_global_scope = std::make_shared<visitor::Scope>(nullptr);

	while (true) {
		std::string input_line;
		std::string prog_name = "REPL";
		std::string source;
		bool file_load = false;
		bool expr = false;

		input_line = read(">>> ");
		input_line = std::regex_replace(input_line, std::regex("^ +| +$"), "$1");

		if (input_line == "#quit") {
			break;
		}
		else if (input_line == "#help") {
			std::cout << "\n" << "Welcome to " << NAME << " " << VER << "! \n";
			std::cout << "To use this interactive REPL, just type in regular Flexa commands and hit\n";
			std::cout << "enter. You can also make use of the following commands: \n\n";

			std::cout << " #load \"file path\"  Loads variable and function declarations from a specified\n";
			std::cout << std::setw(20);
			std::cout << "" << "file into memory, e.g.\n";
			std::cout << std::setw(20);
			std::cout << "" << ">>> #load .\\main.flx\n\n";

			std::cout << " #quit              Exits the Flexa REPL.\n";
			std::cout << std::setw(20);
			std::cout << "" << "functions and variables in the global scope.\n\n";

			std::cout << " #clear             Clears the terminal window.\n\n";
		}
		else if (input_line.starts_with("#load")) {
			if (input_line.size() <= 6) {
				std::cout << "File path expected after '#load'." << std::endl;
				continue;
			}

			std::string file_path = input_line.substr(6);
			source = load_source(file_path);
			prog_name = get_prog_name(file_path);
			file_load = true;
		}
		else if (input_line == "#clear") {
			clear_screen();
		}
		else {
			source += input_line;

			unsigned int open_scopes = 0;
			count_scopes(input_line, open_scopes);

			while (open_scopes) {
				input_line.clear();
				input_line = read("... ");
				count_scopes(input_line, open_scopes);
				source += input_line + "\n";
			}
		}

		try {
			lexer::Lexer lexer(prog_name, source);
			parser::Parser parser(prog_name, &lexer);
			std::shared_ptr<ASTProgramNode> program;
			std::map<std::string, std::shared_ptr<ASTProgramNode>> programs;

			try {
				program = parser.parse_program();
				programs = std::map<std::string, std::shared_ptr<ASTProgramNode>>({ std::pair(prog_name, program) });
			}
			catch (const std::exception& e) {
				std::string err = e.what();
				remove_header(err);
				std::cerr << utils::StringUtils::trim(err) << std::endl;
				continue;
			}

			semantic_global_scope->owner = program;
			interpreter_global_scope->owner = program;

			// check if it's all ok using a temp global scope
			std::shared_ptr<visitor::Scope> temp = std::make_shared<visitor::Scope>(*semantic_global_scope);
			visitor::SemanticAnalyser temp_semantic_analyser(temp, program, programs, args.program_args);
			temp_semantic_analyser.start();

			visitor::SemanticAnalyser semantic_analyser(semantic_global_scope, program, programs, args.program_args);
			semantic_analyser.start();

			visitor::Interpreter interpreter(interpreter_global_scope, program, programs, args.program_args);
			interpreter.visit(program);

			if (file_load) {
				std::cout << std::endl << "File loaded successfully." << std::endl;
			}
			else {
				// not is undefined and it's an expression
				if (!parser::is_undefined(interpreter.current_expression_value->type)
					&& source.find(';') == std::string::npos) {
					std::cout << RuntimeOperations::parse_value_to_string(interpreter.current_expression_value) << std::endl;
				}
			}
		}
		catch (const std::exception& e) {
			std::string err = e.what();
			remove_header(err);
			std::cerr << utils::StringUtils::trim(err) << std::endl;
		}
	}

	return EXIT_SUCCESS;
}
