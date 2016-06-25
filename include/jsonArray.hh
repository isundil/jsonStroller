#pragma once

#include <list>
#include "jsonContainer.hh"

class JSonArray: public JSonContainer, public std::list<JSonElement *>
{
    public:
        JSonArray(JSonContainer *parent);
        virtual ~JSonArray();
        virtual unsigned int size() const;

        virtual JSonElement *firstChild();
        virtual const JSonElement *firstChild() const;
};

