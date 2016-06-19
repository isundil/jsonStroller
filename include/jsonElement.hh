#pragma once

#include <string>

class JSonContainer;

class JSonElement
{
    public:
        JSonElement(JSonContainer *parent);
        virtual ~JSonElement();

        virtual std::string stringify() const =0;
        unsigned int getLevel() const;
        JSonContainer *getParent();
        const JSonContainer *getParent() const;

    private:
        JSonElement();
        JSonContainer *parent;
};

