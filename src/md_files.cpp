#include "md_files.hpp"

#include "interpreter.hpp"
#include "semantic_analysis.hpp"

using namespace modules;
using namespace visitor;

ModuleFiles::ModuleFiles() {}

ModuleFiles::~ModuleFiles() = default;

void ModuleFiles::register_functions(visitor::SemanticAnalyser* visitor) {
	visitor->builtin_functions["open"] = nullptr;
	visitor->builtin_functions["read"] = nullptr;
	visitor->builtin_functions["read_line"] = nullptr;
	visitor->builtin_functions["read_all_bytes"] = nullptr;
	visitor->builtin_functions["write"] = nullptr;
	visitor->builtin_functions["write_bytes"] = nullptr;
	visitor->builtin_functions["is_open"] = nullptr;
	visitor->builtin_functions["close"] = nullptr;
	visitor->builtin_functions["del_file"] = nullptr;
	visitor->builtin_functions["create_file"] = nullptr;
	visitor->builtin_functions["create_folder"] = nullptr;
	visitor->builtin_functions["del_folder"] = nullptr;
	visitor->builtin_functions["path_exists"] = nullptr;
}

void ModuleFiles::register_functions(visitor::Interpreter* visitor) {

	visitor->builtin_functions["open"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("mode"))->value
		};

		// initialize file struct values
		RuntimeValue* cpfile = visitor->alocate_value(new RuntimeValue(parser::Type::T_STRUCT));

		flx_struct str = flx_struct();
		str["path"] = visitor->alocate_value(new RuntimeValue(vals[0]));
		str["mode"] = visitor->alocate_value(new RuntimeValue(vals[1]));

		int parmode = vals[1]->get_i();

		std::fstream* fs = nullptr;
		try {
			fs = new std::fstream(vals[0]->get_s(), parmode);
			str[INSTANCE_ID_NAME] = visitor->alocate_value(new RuntimeValue(flx_int(fs)));
			cpfile->set(str, "File", language_namespace);
			visitor->current_expression_value = cpfile;
		}
		catch (std::exception ex) {
			throw std::runtime_error(ex.what());
		}
		};

	visitor->builtin_functions["read"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->value;

		if (!parser::is_void(val->type)) {
			auto rval = visitor->alocate_value(new RuntimeValue(parser::Type::T_STRING));

			std::fstream* fs = ((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i());

			fs->seekg(0);

			std::stringstream ss;
			std::string line;
			while (std::getline(*fs, line)) {
				ss << line << std::endl;
			}
			rval->set(ss.str());

			visitor->current_expression_value = rval;
		}
		};

	visitor->builtin_functions["read_line"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->value;

		if (!parser::is_void(val->type)) {
			auto rval = visitor->alocate_value(new RuntimeValue(parser::Type::T_STRING));

			std::fstream* fs = ((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i());

			std::string line;
			std::getline(*fs, line);
			rval->set(line);

			visitor->current_expression_value = rval;
		}
		};

	visitor->builtin_functions["read_all_bytes"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->value;

		if (!parser::is_void(val->type)) {
			auto rval = visitor->alocate_value(new RuntimeValue(parser::Type::T_ARRAY));
			rval->set_arr_type(parser::Type::T_CHAR);

			std::fstream* fs = ((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i());

			fs->seekg(0);

			// find file size
			fs->seekg(0, std::ios::end);
			std::streamsize buffer_size = fs->tellg();
			fs->seekg(0, std::ios::beg);

			// buffer to store readed data
			char* buffer = new char[buffer_size];

			flx_array arr = flx_array(buffer_size);

			// read all bytes
			if (fs->read(buffer, buffer_size)) {
				for (size_t i = 0; i < buffer_size; ++i) {
					RuntimeValue* val = visitor->alocate_value(new RuntimeValue(parser::Type::T_CHAR));
					val->set(buffer[i]);
					arr[i] = val;
				}
			}
			rval->set(arr, Type::T_CHAR, std::vector<std::shared_ptr<ASTExprNode>>());

			delete[] buffer;

			visitor->current_expression_value = rval;
		}
		};

	visitor->builtin_functions["write"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("data"))->value
		};

		RuntimeValue* cpfile = vals[0];
		if (!parser::is_void(cpfile->type)) {
			std::fstream* fs = ((std::fstream*)cpfile->get_str()[INSTANCE_ID_NAME]->get_i());
			*fs << vals[1]->get_s();
		}
		};

	visitor->builtin_functions["write_bytes"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->value,
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("bytes"))->value
		};

		RuntimeValue* cpfile = vals[0];
		if (!parser::is_void(cpfile->type)) {
			std::fstream* fs = ((std::fstream*)cpfile->get_str()[INSTANCE_ID_NAME]->get_i());

			auto arr = vals[1]->get_arr();

			std::streamsize buffer_size = arr.size();

			char* buffer = new char[buffer_size];

			for (size_t i = 0; i < buffer_size; ++i) {
				buffer[i] = arr[i]->get_c();
			}

			fs->write(buffer, sizeof(buffer));
		}
		};

	visitor->builtin_functions["is_open"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->value;

		if (!parser::is_void(val->type)) {
			auto rval = visitor->alocate_value(new RuntimeValue(parser::Type::T_BOOL));
			rval->set(flx_bool(((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i())->is_open()));
			visitor->current_expression_value = rval;
		}
		};

	visitor->builtin_functions["close"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->value;

		if (!parser::is_void(val->type)) {
			if (((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i())) {
				((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i())->close();
				((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i())->~basic_fstream();
				val->set_null();
			}
		}
		};

	visitor->builtin_functions["del_file"] = [this, visitor]() {
		throw std::runtime_error("'del_file' was not implemented yet");
		};

	visitor->builtin_functions["create_file"] = [this, visitor]() {
		throw std::runtime_error("'create_file' was not implemented yet");
		};

	visitor->builtin_functions["del_folder"] = [this, visitor]() {
		throw std::runtime_error("'del_folder' was not implemented yet");
		};

	visitor->builtin_functions["create_folder"] = [this, visitor]() {
		throw std::runtime_error("'create_folder' was not implemented yet");
		};

	visitor->builtin_functions["path_exists"] = [this, visitor]() {
		throw std::runtime_error("'path_exists' was not implemented yet");
		};

}

void ModuleFiles::register_functions(visitor::Compiler* visitor) {}

void ModuleFiles::register_functions(vm::VirtualMachine* vm) {}
