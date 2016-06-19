#include "jsonElement.hh"
#include "jsonContainer.hh"

JSonElement::JSonElement(JSonContainer *p): parent(p)
{ }

JSonElement::~JSonElement()
{ }

unsigned int JSonElement::getLevel() const
{
    unsigned int level = 0;
    for (const JSonElement *parent = this; parent; parent = parent->parent)
        level++;
    return level;
}

JSonContainer *JSonElement::getParent()
{
    return parent;
}

const JSonContainer *JSonElement::getParent() const
{
    return parent;
}

