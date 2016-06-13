#pragma once

#include <list>
#include "jsonContainer.hh"

class JSonArray: public JSonContainer, public std::list<JSonElement *>
{
    public:
        virtual ~JSonArray();
        virtual unsigned int size() const;
};

