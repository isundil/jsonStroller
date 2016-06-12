#include "jsonArray.hh"

JSonArray::~JSonArray()
{
    for (iterator i = begin(); i != end(); ++i)
    {
        delete *i;
    }
}

