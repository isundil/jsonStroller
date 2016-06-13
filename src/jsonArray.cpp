#include "jsonArray.hh"

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

