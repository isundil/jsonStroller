#pragma once

#include <string>

class JSonElement
{
    public:
        virtual ~JSonElement();

        virtual std::string stringify() const =0;
};

