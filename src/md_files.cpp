#include "md_files.hpp"

#include <filesystem>

#include "interpreter.hpp"
#include "semantic_analysis.hpp"
#include "constants.hpp"

using namespace core::modules;
using namespace core::runtime;
using namespace core::analysis;

ModuleFiles::ModuleFiles() {}

ModuleFiles::~ModuleFiles() = default;

void ModuleFiles::register_functions(SemanticAnalyser* visitor) {
	visitor->builtin_functions["open"] = nullptr;
	visitor->builtin_functions["read"] = nullptr;
	visitor->builtin_functions["read_line"] = nullptr;
	visitor->builtin_functions["read_all_bytes"] = nullptr;
	visitor->builtin_functions["write"] = nullptr;
	visitor->builtin_functions["write_bytes"] = nullptr;
	visitor->builtin_functions["is_open"] = nullptr;
	visitor->builtin_functions["close"] = nullptr;

	visitor->builtin_functions["is_file"] = nullptr;
	visitor->builtin_functions["is_dir"] = nullptr;

	visitor->builtin_functions["create_dir"] = nullptr;
	visitor->builtin_functions["list_dir"] = nullptr;

	visitor->builtin_functions["path_exists"] = nullptr;
	visitor->builtin_functions["delete_path"] = nullptr;
}

void ModuleFiles::register_functions(Interpreter* visitor) {

	visitor->builtin_functions["open"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("mode"))->get_value()
		};

		// initialize file struct values
		RuntimeValue* cpfile = visitor->allocate_value(new RuntimeValue(Type::T_STRUCT));

		flx_struct str = flx_struct();
		str["path"] = visitor->allocate_value(new RuntimeValue(vals[0]));
		str["mode"] = visitor->allocate_value(new RuntimeValue(vals[1]));

		int parmode = vals[1]->get_i();

		std::fstream* fs = nullptr;
		try {
			std::ios_base::openmode mode = std::ios_base::openmode(parmode);
			fs = new std::fstream(vals[0]->get_s(), mode);
			str[INSTANCE_ID_NAME] = visitor->allocate_value(new RuntimeValue(flx_int(fs)));
			cpfile->set(str, "File", Constants::STD_NAMESPACE);
			visitor->current_expression_value = cpfile;
		}
		catch (std::runtime_error ex) {
			throw std::runtime_error(ex.what());
		}

		};

	visitor->builtin_functions["read"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->get_value();

		if (!TypeUtils::is_void(val->type)) {
			auto rval = visitor->allocate_value(new RuntimeValue(Type::T_STRING));

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
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->get_value();

		if (!TypeUtils::is_void(val->type)) {
			auto rval = visitor->allocate_value(new RuntimeValue(Type::T_STRING));

			std::fstream* fs = ((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i());

			std::string line;
			std::getline(*fs, line);
			rval->set(line);

			visitor->current_expression_value = rval;
		}

		};

	visitor->builtin_functions["read_all_bytes"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->get_value();

		if (!TypeUtils::is_void(val->type)) {
			auto rval = visitor->allocate_value(new RuntimeValue(Type::T_ARRAY));
			rval->set_arr_type(Type::T_CHAR);

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
				for (std::streamsize i = 0; i < buffer_size; ++i) {
					RuntimeValue* val = visitor->allocate_value(new RuntimeValue(Type::T_CHAR));
					val->set(buffer[i]);
					arr[i] = val;
				}
			}
			rval->set(arr, Type::T_CHAR, std::vector<size_t>{(size_t)arr.size()});

			delete[] buffer;

			visitor->current_expression_value = rval;
		}

		};

	visitor->builtin_functions["write"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("data"))->get_value()
		};

		RuntimeValue* cpfile = vals[0];
		if (!TypeUtils::is_void(cpfile->type)) {
			std::fstream* fs = ((std::fstream*)cpfile->get_str()[INSTANCE_ID_NAME]->get_i());
			*fs << vals[1]->get_s();
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["write_bytes"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto vals = std::vector{
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->get_value(),
			std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("bytes"))->get_value()
		};

		RuntimeValue* cpfile = vals[0];
		if (!TypeUtils::is_void(cpfile->type)) {
			std::fstream* fs = ((std::fstream*)cpfile->get_str()[INSTANCE_ID_NAME]->get_i());

			auto arr = vals[1]->get_arr();

			std::streamsize buffer_size = arr.size();

			char* buffer = new char[buffer_size];

			for (std::streamsize i = 0; i < buffer_size; ++i) {
				buffer[i] = arr[i]->get_c();
			}

			fs->write(buffer, sizeof(buffer));
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["is_open"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->get_value();

		if (!TypeUtils::is_void(val->type)) {
			auto rval = visitor->allocate_value(new RuntimeValue(Type::T_BOOL));
			rval->set(flx_bool(((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i())->is_open()));
			visitor->current_expression_value = rval;
		}

		};

	visitor->builtin_functions["close"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("file"))->get_value();

		if (!TypeUtils::is_void(val->type)) {
			if (((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i())) {
				((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i())->close();
				((std::fstream*)val->get_str()[INSTANCE_ID_NAME]->get_i())->~basic_fstream();
				val->set_null();
			}
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["is_file"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->get_value();

		std::filesystem::path path = val->get_s();

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(std::filesystem::is_regular_file(path)));

		};

	visitor->builtin_functions["is_dir"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->get_value();

		std::filesystem::path path = val->get_s();

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(std::filesystem::is_directory(path)));

		};

	visitor->builtin_functions["create_dir"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->get_value();

		std::filesystem::path path = val->get_s();

		if (!std::filesystem::create_directories(path)) {
			throw std::runtime_error("cannot create directory");
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

	visitor->builtin_functions["list_dir"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->get_value();

		std::filesystem::path path = val->get_s();

		std::vector<std::string> files;

		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			files.push_back(entry.path().filename().string());
		}

		flx_array values = flx_array(files.size());

		for (size_t i = 0; i < files.size(); ++i) {
			const auto& file = files[i];
			values[i] = visitor->allocate_value(new RuntimeValue(file));
		}

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(values));

		};

	visitor->builtin_functions["path_exists"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->get_value();

		std::filesystem::path path = val->get_s();

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(std::filesystem::exists(path)));

		};

	visitor->builtin_functions["delete_path"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("path"))->get_value();

		std::filesystem::path path = val->get_s();

		std::filesystem::remove_all(path);

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(Type::T_UNDEFINED));

		};

}
