/**
 * jsonContainer.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "jsonContainer.hh"
#include "levenshtein.hh"

JSonContainer::JSonContainer(JSonContainer *p):JSonElement(p)
{ }

JSonContainer::~JSonContainer()
{ }

float JSonContainer::diff(const JSonElement *other) const
{
    if (!dynamic_cast<const JSonContainer *> (other))
        return 0.f;
    return levenshteinPercent<JSonElement>(this, (const JSonContainer*)other);
}

