#pragma once

#include "jsonElement.hh"

class JSonObject;

class JSonObjectEntry: public JSonElement
{
    public:
        JSonObjectEntry(JSonObject*, const std::string &key, JSonElement *item);
        ~JSonObjectEntry();

        std::string stringify() const;

        bool operator==(const std::string &) const;
        const JSonElement *operator*() const;
        JSonElement *operator*();

    protected:
        const std::string key;
        JSonElement * const value;
};

