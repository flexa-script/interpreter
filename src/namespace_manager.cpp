#include "namespace_manager.hpp"

#include "constants.hpp"

using namespace core;

const std::string& NamespaceManager::get_namespace() const {
	if (current_namespace.empty()) {
		return Constants::DEFAULT_NAMESPACE;
	}
	return current_namespace.top();
}
