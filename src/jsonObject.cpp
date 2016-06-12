#include "jsonObject.hh"
#include "jsonPrimitive.hh"

JSonObject::JSonObject()
{
    children = new std::map<JSonPrimitive<std::string>, JSonElement *>();
}

void JSonObject::push(const JSonPrimitive<std::string> &key, JSonElement *child)
{
    (*children)[key] = child;
}

bool JSonObject::contains(const JSonPrimitive<std::string> &key) const
{
    return children->find(key) != children->end();
}

