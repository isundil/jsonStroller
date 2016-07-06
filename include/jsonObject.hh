#pragma once

#include "jsonContainer.hh"
#include "jsonObjectEntry.hh"

class JSonObject: public JSonContainer
{
    public:
        JSonObject(JSonContainer *parent);
        virtual ~JSonObject();

        void push(const std::string &key, JSonElement *child);
        JSonObject::const_iterator find(const std::string &key) const;
        bool contains(const std::string &) const;

        virtual JSonElement *firstChild();
        virtual const JSonElement *firstChild() const;

        const JSonElement* get(const std::string &) const;

        virtual std::string stringify() const;
};

