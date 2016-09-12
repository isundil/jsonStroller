/**
 * jsonElement.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <string>

class JSonContainer;
class SearchPattern;

class JSonElement
{
    public:
        JSonElement(JSonElement *parent);
        virtual ~JSonElement();

        /**
         * return string-representative of this JSonElement
        **/
        virtual std::string stringify() const =0;

        virtual float diff(const JSonElement *) const;

        /**
         * get the number of col string will output
        **/
        virtual size_t strlen() const;
        virtual size_t lazystrlen();

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
        virtual bool match(const SearchPattern &) const;
        virtual bool operator==(const JSonElement *other) const;

    private:
        JSonElement();
        /**
         * parent container
         * Not a JSonContainer because JSonObjectEntry can be a parent and isn't a JSonContainer either
        **/
        JSonElement *parent;

        size_t _strlen;
};

