#include "jsonArray.hh"

JSonArray::JSonArray(JSonContainer *p): JSonContainer(p)
{ }

JSonArray::~JSonArray()
{
    for (iterator i = begin(); i != end(); ++i)
    {
        delete *i;
    }
}

unsigned int JSonArray::size() const
{
    return std::list<JSonElement *>::size();
}

JSonElement *JSonArray::firstChild()
{
    if (begin() == end())
        return nullptr;
    return *begin();
}

const JSonElement *JSonArray::firstChild() const
{
    if (cbegin() == cend())
        return nullptr;
    return *cbegin();
}

