#include "jsonObject.hh"
#include "jsonPrimitive.hh"

JSonObject::JSonObject(JSonContainer *p): JSonContainer(p)
{ }

JSonObject::~JSonObject()
{
    for (iterator i = begin(); i != end(); ++i)
    {
        delete (*i).second;
    }
}

void JSonObject::push(const std::string &key, JSonElement *child)
{
    (*this)[key] = child;
}

bool JSonObject::contains(const std::string &key) const
{
    return find(key) != end();
}

const JSonElement *JSonObject::get(const std::string &key) const
{
    const_iterator item = find(key);
    return item == cend() ? nullptr : (*item).second;
}

unsigned int JSonObject::size() const
{
    return std::map<std::string, JSonElement *>::size();
}

JSonElement *JSonObject::firstChild()
{
    if (begin() == end())
        return nullptr;
    return (*begin()).second;
}

const JSonElement *JSonObject::firstChild() const
{
    if (cbegin() == cend())
        return nullptr;
    return (*cbegin()).second;
}

std::string JSonObject::stringify() const
{
    return "{ }";
}

