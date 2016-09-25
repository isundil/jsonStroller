/**
 * jsonContainer.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "jsonContainer.hh"

JSonContainer::JSonContainer(JSonContainer *p):JSonElement(p)
{ }

JSonContainer::~JSonContainer()
{ }

bool JSonContainer::operator==(const JSonElement *other) const
{
    if (!dynamic_cast<const JSonContainer *> (other) || size() != ((const JSonContainer*)other)->size())
        return false;
    const_iterator a = cbegin();
    const_iterator b = ((const JSonContainer*)other)->cbegin();
    while (a != cend() && b != ((const JSonContainer*)other)->cend())
    {
        if (*a != *b)
            return false;
        a++;
        b++;
    }
    return true;
}

