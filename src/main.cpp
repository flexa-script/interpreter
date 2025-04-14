#ifdef linux
#include <locale.h>
#elif defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#endif // linux

#include "flx_repl.hpp"
#include "flx_interpreter.hpp"
#include "watch.hpp"

using namespace interpreter;

int main(int argc, const char* argv[]) {
#ifdef linux
	setlocale(LC_ALL, "en_US.UTF-8");
#elif defined(_WIN32) || defined(WIN32)
	SetConsoleOutputCP(CP_UTF8);
#endif // linux

	auto args = FlexaCliArgs(argc, argv);

	if (!args.main_file.empty() && args.workspace_path.empty()) {
		throw std::runtime_error("workspace must be informed");
	}

	if (args.main_file.empty() && !args.workspace_path.empty()) {
		throw std::runtime_error("main file must be informed");
	}

	if (args.source_files.size() > 0 && args.workspace_path.empty()) {
		throw std::runtime_error("workspace must be informed");
	}

	if (args.debug) {
#ifndef __FLX_DEBUG__
#define __FLX_DEBUG__
#endif // !__FLX_DEBUG__
	}

	if (args.workspace_path.empty()) {
		return FlexaRepl::execute(args);
	}

	auto interpreter = FlexaInterpreter(args);

	auto sw = utils::ChronoStopwatch();
	sw.start();
	int result = interpreter.execute();
	sw.stop();

	if (args.debug) {
		std::cout << std::endl << "execution time: " << sw.get_elapsed_formatted() << std::endl;
		std::cout << "process finished with exit code " << result << std::endl;
		system("pause");
	}

	return result;
}
