#ifndef NAMESPACE_MANAGER_HPP
#define NAMESPACE_MANAGER_HPP

#include <string>
#include <stack>

namespace core {

	class NamespaceManager {
	public:
		std::stack<std::string> current_namespace;

		NamespaceManager() = default;
		virtual ~NamespaceManager() = default;

		virtual bool push_namespace(const std::string name_space) = 0;
		virtual void pop_namespace(bool pop) = 0;

		const std::string& get_namespace() const;
	};

}

#endif // !NAMESPACE_MANAGER_HPP
