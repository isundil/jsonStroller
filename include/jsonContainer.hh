#pragma once

#include "jsonElement.hh"

class JSonContainer: public JSonElement
{
    public:
        virtual ~JSonContainer();
        virtual unsigned int size() const =0;

        virtual std::string stringify() const;
};

