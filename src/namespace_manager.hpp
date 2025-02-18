#ifndef NAMESPACE_MANAGER_HPP
#define NAMESPACE_MANAGER_HPP

#include <string>

namespace visitor {

	class NamespaceManager {
	public:
		NamespaceManager() = default;
		virtual ~NamespaceManager() = default;
		virtual bool push_namespace(const std::string name_space) = 0;
		virtual void pop_namespace(bool pop) = 0;
	};
}

#endif // !NAMESPACE_MANAGER_HPP
