#pragma once

#include <list>
#include "jsonElement.hh"

class JSonArray: public JSonElement, public std::list<JSonElement *>
{
    public:
        virtual ~JSonArray();
};

