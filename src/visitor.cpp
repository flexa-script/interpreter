#include "visitor.hpp"

#include "md_builtin.hpp"
#include "md_datetime.hpp"
#include "md_graphics.hpp"
#include "md_files.hpp"
#include "md_console.hpp"
#include "md_input.hpp"
#include "md_sound.hpp"
#include "md_HTTP.hpp"

using namespace visitor;

Visitor::Visitor(const std::map<std::string, std::shared_ptr<ASTProgramNode>>& programs, std::shared_ptr<ASTProgramNode> main_program, const std::string& current_this_name)
	: programs(programs), main_program(main_program), curr_row(0), curr_col(0) {
	current_program.push(main_program);
};

std::string default_namespace = "__default__";

std::vector<std::string> std_libs = {
	"flx.std.collections.collection",
	"flx.std.collections.dictionary",
	"flx.std.collections.hashtable",
	"flx.std.collections.list",
	"flx.std.collections.queue",
	"flx.std.collections.stack",
	"flx.std.arrays",
	"flx.std.math",
	"flx.std.print",
	"flx.std.random",
	"flx.std.strings",
	"flx.std.structs",
	"flx.std.testing",
	"flx.std.utils",
	"flx.std.DSL.BPS",
	"flx.std.DSL.JSON",
	"flx.std.DSL.YAML",
	"flx.std.DSL.XML"
};

std::map<std::string, std::shared_ptr<modules::Module>> built_in_libs = {
	{"builtin", std::shared_ptr<modules::ModuleBuiltin>(new modules::ModuleBuiltin())},
	{"flx.core.graphics", std::shared_ptr<modules::ModuleGraphics>(new modules::ModuleGraphics())},
	{"flx.core.files", std::shared_ptr<modules::ModuleFiles>(new modules::ModuleFiles())},
	{"flx.core.console", std::shared_ptr<modules::ModuleConsole>(new modules::ModuleConsole())},
	{"flx.core.datetime", std::shared_ptr<modules::ModuleDateTime>(new modules::ModuleDateTime())},
	{"flx.core.input", std::shared_ptr<modules::ModuleInput>(new modules::ModuleInput())},
	{"flx.core.sound", std::shared_ptr<modules::ModuleSound>(new modules::ModuleSound())},
	{"flx.core.HTTP", std::shared_ptr<modules::ModuleHTTP>(new modules::ModuleHTTP())}
};
