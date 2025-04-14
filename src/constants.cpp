#include "constants.hpp"

#include "md_builtin.hpp"
#include "md_datetime.hpp"
#include "md_graphics.hpp"
#include "md_files.hpp"
#include "md_gc.hpp"
#include "md_console.hpp"
#include "md_input.hpp"
#include "md_sound.hpp"
#include "md_http.hpp"

using namespace core;

std::string const Constants::NAME = "Flexa";

std::string const Constants::VER = "v1.4.1";

std::string const Constants::YEAR = "2025";

std::string const Constants::STD_NAMESPACE = "flx";

std::string const Constants::DEFAULT_NAMESPACE = "__default__";

std::unordered_map<BuintinFuncs, std::string> const Constants::BUILTIN_NAMES = {
	{PRINT, "print"},
	{PRINTLN, "println"},
	{READ, "read"},
	{READCH, "readch"},
	{LEN, "len"},
	{SLEEP, "sleep"},
	{SYSTEM, "system"}
};

std::vector<std::string> const Constants::STD_LIBS = {
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
	"flx.std.types",
	"flx.std.testing",
	"flx.std.utils",
	"flx.std.DSL.FML",
	"flx.std.DSL.JSON",
	"flx.std.DSL.YAML",
	"flx.std.DSL.XML"
};

std::unordered_map<std::string, std::shared_ptr<modules::Module>> const Constants::BUILT_IN_LIBS = {
	{"builtin", std::shared_ptr<modules::ModuleBuiltin>(new modules::ModuleBuiltin())},
	{"flx.core.gc", std::shared_ptr<modules::ModuleGC>(new modules::ModuleGC())},
	{"flx.core.graphics", std::shared_ptr<modules::ModuleGraphics>(new modules::ModuleGraphics())},
	{"flx.core.files", std::shared_ptr<modules::ModuleFiles>(new modules::ModuleFiles())},
	{"flx.core.console", std::shared_ptr<modules::ModuleConsole>(new modules::ModuleConsole())},
	{"flx.core.datetime", std::shared_ptr<modules::ModuleDateTime>(new modules::ModuleDateTime())},
	{"flx.core.input", std::shared_ptr<modules::ModuleInput>(new modules::ModuleInput())},
	{"flx.core.sound", std::shared_ptr<modules::ModuleSound>(new modules::ModuleSound())},
	{"flx.core.HTTP", std::shared_ptr<modules::ModuleHTTP>(new modules::ModuleHTTP())}
};
