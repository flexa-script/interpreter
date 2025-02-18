#include "gc.hpp"

using namespace gc;

GarbageCollector::GarbageCollector() {}

GarbageCollector::~GarbageCollector() {
	for (GCObject* obj : heap) {
		if (obj) {
			delete obj;
		}
	}
}

GCObject* GarbageCollector::allocate(GCObject* obj) {
	heap.push_back(obj);
	return obj;
}

void GarbageCollector::add_root(GCObject* obj) {
	roots.push_back(obj);
}

void GarbageCollector::remove_root(GCObject* obj) {
	auto it = std::find(roots.begin(), roots.end(), obj);
	if (it != roots.end()) {
		roots.erase(it);
	}
}

void GarbageCollector::add_ptr_root(RuntimeValue** obj) {
	ptr_roots.push_back(obj);
}

void GarbageCollector::remove_ptr_root(RuntimeValue** obj) {
	auto it = std::find(ptr_roots.begin(), ptr_roots.end(), obj);
	if (it != ptr_roots.end()) {
		ptr_roots.erase(it);
	}
}

void GarbageCollector::add_var_root(std::weak_ptr<GCObject> obj) {
	var_roots.push_back(obj);
}

void GarbageCollector::remove_var_root(std::weak_ptr<GCObject> obj) {
	if (auto obj_ptr = obj.lock()) {
		auto it = std::find_if(var_roots.begin(), var_roots.end(), [&obj_ptr](const std::weak_ptr<GCObject>& root) {
			if (auto root_ptr = root.lock()) {
				return root_ptr == obj_ptr;
			}
			return false;
			});

		if (it != var_roots.end()) {
			var_roots.erase(it);
		}
	}
}

void GarbageCollector::add_root_container(std::weak_ptr<std::vector<RuntimeValue*>> root_container) {
	root_containers.emplace_back(root_container);
}

void GarbageCollector::remove_root_container(std::weak_ptr<std::vector<RuntimeValue*>> root_container) {
	if (auto obj_ptr = root_container.lock()) {
		auto it = std::find_if(root_containers.begin(), root_containers.end(), [&obj_ptr](const std::weak_ptr<std::vector<RuntimeValue*>>& root) {
			if (auto root_ptr = root.lock()) {
				return root_ptr == obj_ptr;
			}
			return false;
			});

		if (it != root_containers.end()) {
			root_containers.erase(it);
		}
	}
}

void GarbageCollector::mark() {
	for (auto it = roots.begin(); it != roots.end();) {
		if (*it) {
			mark_object(*it);
			++it;
		}
		else {
			it = roots.erase(it);
		}
	}

	for (auto it = ptr_roots.begin(); it != ptr_roots.end();) {
		if (*it) {
			mark_object(**it);
			++it;
		}
		else {
			it = ptr_roots.erase(it);
		}
	}

	for (auto it = var_roots.begin(); it != var_roots.end();) {
		if (auto root = it->lock()) {
			mark_object(root.get());
			++it;
		}
		else {
			it = var_roots.erase(it);
		}
	}

	for (auto it = root_containers.begin(); it != root_containers.end();) {
		if (auto root = it->lock()) {
			for (auto item : *root.get()) {
				mark_object(item);
			}
			++it;
		}
		else {
			it = root_containers.erase(it);
		}
	}
}

void GarbageCollector::mark_object(GCObject* obj) {
	if (obj == nullptr || obj->marked) return;
	obj->marked = true;

	for (GCObject* referenced : obj->get_references()) {
		mark_object(referenced);
	}
}

void GarbageCollector::sweep() {
	for (auto it = heap.begin(); it != heap.end(); ) {
		if (!(*it)->marked) {
			delete* it;
			it = heap.erase(it);
		}
		else {
			(*it)->marked = false;
			++it;
		}
	}

	for (auto it = roots.begin(); it != roots.end(); ++it) {
		(*it)->marked = false;
	}

	for (auto it = var_roots.begin(); it != var_roots.end(); ++it) {
		if (auto root = it->lock()) {
			root->marked = false;
		}
	}

	for (const auto& iterable_ptr : root_containers) {
		if (auto root = iterable_ptr.lock()) {
			for (auto item : *root) {
				item->marked = false;
			}
		}
	}
}

void GarbageCollector::collect() {
	mark();
	sweep();
}
