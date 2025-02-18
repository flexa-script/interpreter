#include "flx_repl.hpp"
#include "flx_interpreter.hpp"
#include "watch.hpp"

#include <Windows.h>

int main(int argc, const char* argv[]) {
	SetConsoleOutputCP(CP_UTF8);

	auto args = parse_args(argc, argv);

	if (argc == 1) {
		return FlexaRepl::execute(args);
	}

	if (args.debug) {
#ifndef __FLX_DEBUG__
#define __FLX_DEBUG__
#endif // !__FLX_DEBUG__
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
