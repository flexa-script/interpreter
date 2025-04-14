#ifndef GARBAGE_COLLECTOR_HPP
#define GARBAGE_COLLECTOR_HPP

#include <vector>

#include "gcobject.hpp"
#include "types.hpp"

namespace core {

	namespace runtime {

		class GarbageCollector {
		private:
			std::vector<GCObject*> heap;
			std::vector<GCObject*> roots;
			std::vector<RuntimeValue**> ptr_roots;
			std::vector<std::weak_ptr<GCObject>> var_roots;
			std::vector<std::weak_ptr<std::vector<RuntimeValue*>>> root_containers;
			std::vector<std::weak_ptr<flx_array>> array_roots;

		public:
			bool enable = true;
			size_t max_heap = 9999;

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

			void add_array_root(std::weak_ptr<flx_array> array_root);
			void remove_array_root(std::weak_ptr<flx_array> array_root);

			void mark();
			void mark_object(GCObject* obj);
			void sweep();
			void collect();

		};

	}

}

#endif // !GARBAGE_COLLECTOR_HPP
