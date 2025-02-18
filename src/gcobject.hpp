#ifndef GCOBJECT_HPP
#define GCOBJECT_HPP

#include <vector>

namespace gc {

    class GCObject {
    public:
        bool marked = false;
        virtual ~GCObject();
        virtual std::vector<GCObject*> get_references() = 0;
    };

}

#endif // !GCOBJECT_HPP
