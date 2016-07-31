/**
 * jsonObject.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include "config.h"
#include "jsonContainer.hh"
#include "jsonObjectEntry.hh"
#include "jsonException.hh"

class JSonObject: public JSonContainer
{
    public:
        JSonObject(JSonContainer *parent);
        virtual ~JSonObject();

        void push(const std::string &key, JSonElement *child);
        JSonObject::const_iterator find(const std::string &key) const;
        bool erase(const std::string &);
        bool contains(const std::string &) const;

        virtual JSonElement *firstChild();
        virtual const JSonElement *firstChild() const;

        const JSonElement* get(const std::string &) const;

        virtual std::string stringify() const;

    class DoubleKeyException: public JsonException
    {
        public:
            DoubleKeyException(unsigned long long offset, const std::string &key, LinearHistory &buf);
    };

    class NotAKeyException: public JsonException
    {
        public:
            NotAKeyException(unsigned long long offset, LinearHistory &buf);
    };
};

