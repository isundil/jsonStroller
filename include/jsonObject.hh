#pragma once

#include <map>
#include "jsonElement.hh"

template<typename T> class JSonPrimitive;

class JSonObject: public JSonElement
{
    public:
        JSonObject();
        void push(const JSonPrimitive<std::string> &key, JSonElement *child);

        bool contains(const JSonPrimitive<std::string> &) const;

    protected:
        std::map<JSonPrimitive<std::string>, JSonElement *> *children;
};

