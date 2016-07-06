#include "jsonObject.hh"
#include "jsonPrimitive.hh"

JSonObject::JSonObject(JSonContainer *p): JSonContainer(p)
{ }

JSonObject::~JSonObject()
{
    for (iterator i = begin(); i != end(); ++i)
    {
        delete (*i);
    }
}

void JSonObject::push(const std::string &key, JSonElement *child)
{
    this->push_back(new JSonObjectEntry(this, key, child));
}

JSonObject::const_iterator JSonObject::find(const std::string &key) const
{
    JSonObject::const_iterator it = cbegin();
    while (it != cend())
    {
        if ((const JSonObjectEntry &)(**it) == key)
            return it;
        ++it;
    }
    return it;
}

bool JSonObject::contains(const std::string &key) const
{
    return find(key) != this->cend();
}

const JSonElement *JSonObject::get(const std::string &key) const
{
    const_iterator item = find(key);
    if (item == cend())
        return nullptr;
    return *(const JSonObjectEntry &)(**item);
}

JSonElement *JSonObject::firstChild()
{
    if (begin() == end())
        return nullptr;
    return *begin();
}

const JSonElement *JSonObject::firstChild() const
{
    if (cbegin() == cend())
        return nullptr;
    return *cbegin();
}

std::string JSonObject::stringify() const
{
    return "{ }";
}

