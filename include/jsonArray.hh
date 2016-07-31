/**
 * jsonArray.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include "jsonContainer.hh"

class JSonArray: public JSonContainer
{
    public:
        JSonArray(JSonContainer *parent);
        virtual ~JSonArray();

        virtual JSonElement *firstChild();
        virtual const JSonElement *firstChild() const;

        virtual std::string stringify() const;
};

