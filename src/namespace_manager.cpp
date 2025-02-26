#include "namespace_manager.hpp"

using namespace core;

const std::string& NamespaceManager::get_namespace() const {
	if (current_namespace.empty()) {
		return "";
	}
	return current_namespace.top();
}
