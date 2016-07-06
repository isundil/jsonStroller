
#include "jsonObjectEntry.hh"
#include "jsonObject.hh"

JSonObjectEntry::JSonObjectEntry(JSonObject *parent, const std::string &k, JSonElement *v): JSonElement(parent), key(k), value(v)
{ }

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

