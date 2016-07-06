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

std::string JSonArray::stringify() const
{
    return "[ ]";
}

