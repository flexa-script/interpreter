#ifndef CPINTERPRETER_HPP
#define CPINTERPRETER_HPP

#include "flx_utils.hpp"
#include "semantic_analysis.hpp"

class FlexaInterpreter {
private:
	std::string cp_root;
	std::string project_root;
	FlexaCliArgs args;

public:
	FlexaInterpreter(const FlexaCliArgs& args);

	int execute();

private:
	FlexaSource load_program(const std::string& source);
	std::vector<FlexaSource> load_programs(const std::vector<std::string>& source_files);

	void parse_programs(const std::vector<FlexaSource>& source_programs, std::shared_ptr<ASTProgramNode>* main_program,
		std::map<std::string, std::shared_ptr<ASTProgramNode>>* programs);

	int interpreter();

};

#endif // !CPINTERPRETER_HPP
