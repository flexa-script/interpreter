#ifndef FLX_REPL_HPP
#define FLX_REPL_HPP

#include <string>

#include "lexer.hpp"
#include "parser.hpp"
#include "semantic_analysis.hpp"
#include "interpreter.hpp"
#include "flx_utils.hpp"

#ifdef __unix__
#define clear_screen() system("clear")
#elif defined(_WIN32) || defined(WIN32)
#define clear_screen() system("cls")
#endif // __unix__

namespace interpreter {

	class FlexaRepl {
	private:
		static const std::string NAME;
		static const std::string VER;
		static const std::string YEAR;
	public:
		static void remove_header(std::string& err);
		static void count_scopes(const std::string& input_line, unsigned int& open_scopes);
		static std::string read(const std::string& msg);
		static int execute(const FlexaCliArgs& args);
	};

}

#endif // !FLX_REPL_HPP
