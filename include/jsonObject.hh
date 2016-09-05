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

        /**
         * Add entry
        **/
        virtual void push(const std::string &key, JSonElement *child);

        /**
         * find object by key
        **/
        JSonObject::const_iterator find(const std::string &key) const;
        /**
         * remove object by key
        **/
        bool erase(const std::string &);
        /**
         * check if object exists
        **/
        bool contains(const std::string &) const;

        /**
         * fetch object by key
        **/
        const JSonElement* get(const std::string &) const;

        virtual JSonElement *firstChild();
        virtual const JSonElement *firstChild() const;

        virtual std::string stringify() const;

    /**
     * multiple objects exists for this key
    **/
    class DoubleKeyException: public JsonException
    {
        public:
            DoubleKeyException(unsigned long long offset, const std::string &key, LinearHistory &buf);
    };

    /**
     * Invalid value for key
    **/
    class NotAKeyException: public JsonException
    {
        public:
            NotAKeyException(unsigned long long offset, LinearHistory &buf);
    };
};

class JSonSortedObject: public JSonObject
{
    public:
        JSonSortedObject(JSonContainer *parent);

        void push(const std::string &key, JSonElement *child);
};

