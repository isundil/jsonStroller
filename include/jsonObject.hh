#pragma once

#include <map>
#include "jsonElement.hh"

template<typename T> class JSonPrimitive;

class JSonObject: public JSonElement, public std::map<std::string, JSonElement*>
{
    public:
        virtual ~JSonObject();

        void push(const std::string &key, JSonElement *child);
        bool contains(const std::string &) const;

        const JSonElement* get(const std::string &) const;
};

