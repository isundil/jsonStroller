#pragma once

#include "jsonElement.hh"

class JSonContainer: public JSonElement
{
    public:
        JSonContainer(JSonContainer *parent);
        virtual ~JSonContainer();
        virtual unsigned int size() const =0;

        virtual std::string stringify() const;
};

