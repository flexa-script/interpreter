#ifndef GARBAGE_COLLECTOR_HPP
#define GARBAGE_COLLECTOR_HPP

#include <vector>
#include <unordered_set>
#include <memory>
#include <ranges>
#include <type_traits>
#include <variant>

#include "gcobject.hpp"
#include "types.hpp"

namespace gc {

	class GarbageCollector {
	private:
		std::vector<GCObject*> heap;
		std::vector<GCObject*> roots;
		std::vector<RuntimeValue**> ptr_roots;
		std::vector<std::weak_ptr<GCObject>> var_roots;
		std::vector<std::weak_ptr<std::vector<RuntimeValue*>>> root_containers;

	public:
		GarbageCollector();
		~GarbageCollector();

		GCObject* allocate(GCObject* obj);

		void add_root(GCObject* obj);
		void remove_root(GCObject* obj);

		void add_ptr_root(RuntimeValue** obj);
		void remove_ptr_root(RuntimeValue** obj);

		void add_var_root(std::weak_ptr<GCObject> obj);
		void remove_var_root(std::weak_ptr<GCObject> obj);

		void add_root_container(std::weak_ptr<std::vector<RuntimeValue*>> root_container);
		void remove_root_container(std::weak_ptr<std::vector<RuntimeValue*>> root_container);

		void mark();
		void mark_object(GCObject* obj);
		void sweep();
		void collect();

	};

}

#endif // !GARBAGE_COLLECTOR_HPP
