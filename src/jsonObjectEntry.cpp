/**
 * jsonObjectEntry.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "jsonObjectEntry.hh"
#include "jsonObject.hh"

JSonObjectEntry::JSonObjectEntry(JSonObject *parent, const std::string &k, JSonElement *v): JSonElement(parent), key(k), value(v)
{
    v->setParent(this);
}

JSonObjectEntry::~JSonObjectEntry()
{
    delete value;
}

bool JSonObjectEntry::operator==(const std::string &k) const
{
    return key == k;
}

JSonElement *JSonObjectEntry::operator*()
{
    return value;
}

const JSonElement *JSonObjectEntry::operator*() const
{
    return value;
}

std::string JSonObjectEntry::stringify() const
{
    return key;
}

bool JSonObjectEntry::operator<(const JSonElement *e) const
{
    if (dynamic_cast<const JSonObjectEntry*>(e))
    {
        return ((const JSonObjectEntry*)e)->key < key;
    }
    return false;
}

bool JSonObjectEntry::operator<(const JSonElement &e) const
{ return (*this < &e); }

