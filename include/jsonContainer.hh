#pragma once

#include "jsonElement.hh"

class JSonContainer: public JSonElement
{
    public:
        JSonContainer(JSonContainer *parent);
        virtual ~JSonContainer();
        virtual unsigned int size() const =0;
        virtual JSonElement *firstChild() =0;
        virtual const JSonElement *firstChild() const =0;
};

