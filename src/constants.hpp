#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "module.hpp"

namespace core {

	enum BuintinFuncs {
		PRINT,
		PRINTLN,
		READ,
		READCH,
		LEN,
		SLEEP,
		SYSTEM
	};

	class Constants {
	public:
		std::string static const NAME;
		std::string static const VER;
		std::string static const YEAR;

		std::string static const DEFAULT_NAMESPACE;
		std::string static const STD_NAMESPACE;

		std::unordered_map<BuintinFuncs, std::string> static const BUILTIN_NAMES;
		std::vector<std::string> static const STD_LIBS;
		std::unordered_map<std::string, std::shared_ptr<modules::Module>> static const BUILT_IN_LIBS;

	};

}

#endif // !CONSTANTS_HPP
