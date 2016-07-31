/**
 * jsonElement.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <string>

class JSonContainer;

class JSonElement
{
    public:
        JSonElement(JSonElement *parent);
        virtual ~JSonElement();

        /**
         * return string-representative of this JSonElement
        **/
        virtual std::string stringify() const =0;
        /**
         * return number of parents this item has
        **/
        unsigned int getLevel() const;

        /**
         * Get the parent element
        **/
        JSonElement *getParent();
        const JSonElement *getParent() const;

        /**
         * set parent.
         * Used for lazy-init
        **/
        void setParent(JSonElement *parent);

        /**
         * return previous child
        **/
        const JSonElement *findPrev() const;
        const JSonElement *findNext() const;

        /**
         * check if this element match SearchQuery
        **/
        virtual bool match(const std::string &) const;

    private:
        JSonElement();
        /**
         * parent container
         * Not a JSonContainer because JSonObjectEntry can be a parent and isn't a JSonContainer either
        **/
        JSonElement *parent;
};

