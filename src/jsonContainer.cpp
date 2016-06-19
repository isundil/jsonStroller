#include "jsonContainer.hh"

JSonContainer::JSonContainer(JSonContainer *p):JSonElement(p)
{ }

JSonContainer::~JSonContainer()
{ }

std::string JSonContainer::stringify() const
{
    return std::string();
}

