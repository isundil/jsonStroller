/**
 * jsonObjectEntry.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include "jsonElement.hh"

class JSonObject;

class JSonObjectEntry: public JSonElement
{
    public:
        JSonObjectEntry(JSonObject*, const std::string &key, JSonElement *item);
        virtual ~JSonObjectEntry();

        std::string stringify() const;

        bool operator==(const std::string &) const;
        bool operator==(const JSonElement *) const;
        bool operator<(const JSonElement *) const;
        bool operator<(const JSonElement &) const;
        /**
         * Get associated value
        **/
        const JSonElement *operator*() const;
        JSonElement *operator*();

    protected:
        const std::string key;
        JSonElement * const value;
};

