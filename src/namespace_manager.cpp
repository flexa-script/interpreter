#include "namespace_manager.hpp"

using namespace visitor;

const std::string& NamespaceManager::get_namespace() const {
	if (current_namespace.empty()) {
		return "";
	}
	return current_namespace.top();
}

//bool NamespaceManager::push_namespace(const std::string name_space) {
//	if (!name_space.empty() && name_space != get_namespace()) {
//		current_namespace.push(name_space);
//		return true;
//	}
//	return false;
//}

//void NamespaceManager::pop_namespace(bool pop) {
//	if (pop) {
//		current_namespace.pop();
//	}
//}
