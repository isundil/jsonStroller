#pragma once

#include <string>

class JSonContainer;

class JSonElement
{
    public:
        JSonElement(JSonElement *parent);
        virtual ~JSonElement();

        virtual std::string stringify() const =0;
        unsigned int getLevel() const;
        JSonElement *getParent();
        const JSonElement *getParent() const;

        void setParent(JSonElement *parent);

        const JSonElement *findPrev() const;
        const JSonElement *findNext() const;

    private:
        JSonElement();
        JSonElement *parent;
};

