#pragma once

#include <map>
#include "jsonContainer.hh"

template<typename T> class JSonPrimitive;

class JSonObject: public JSonContainer, public std::map<std::string, JSonElement*>
{
    public:
        JSonObject(JSonContainer *parent);
        virtual ~JSonObject();

        void push(const std::string &key, JSonElement *child);
        virtual unsigned int size() const;
        bool contains(const std::string &) const;

        virtual JSonElement *firstChild();
        virtual const JSonElement *firstChild() const;

        const JSonElement* get(const std::string &) const;
};

