/**
 * jsonContainer.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <list>
#include "jsonElement.hh"

class JSonContainer: public JSonElement, public std::list<JSonElement*>
{
    public:
        JSonContainer(JSonContainer *parent);
        virtual ~JSonContainer();
        virtual JSonElement *firstChild() =0;
        virtual const JSonElement *firstChild() const =0;
};

