#ifndef FLX_INTERPRETER_HPP
#define FLX_INTERPRETER_HPP

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "flx_utils.hpp"
#include "semantic_analysis.hpp"

namespace interpreter {

	class FlexaInterpreter {
	private:
		std::string libs_root;
		std::string project_root;
		FlexaCliArgs args;

	public:
		FlexaInterpreter(const FlexaCliArgs& args);

		int execute();

	private:
		FlexaSource load_program(const std::string& source);
		std::vector<FlexaSource> load_programs(const std::vector<std::string>& source_files);

		void parse_programs(const std::vector<FlexaSource>& source_programs, std::shared_ptr<core::ASTProgramNode>* main_program,
			std::map<std::string, std::shared_ptr<core::ASTProgramNode>>* programs);

		int interpreter();

	};

}

#endif // !FLX_INTERPRETER_HPP
