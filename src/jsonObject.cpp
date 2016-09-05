/**
 * jsonObject.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "jsonObject.hh"

JSonObject::JSonObject(JSonContainer *p): JSonContainer(p)
{ }

JSonSortedObject::JSonSortedObject(JSonContainer *p): JSonObject(p)
{ }

JSonObject::~JSonObject()
{
    for (JSonElement *i : *this)
        delete i;
}

void JSonObject::push(const std::string &key, JSonElement *child)
{
    this->push_back(new JSonObjectEntry(this, key, child));
}

void JSonSortedObject::push(const std::string &key, JSonElement *child)
{
    JSonObject::iterator pos = begin();
    JSonObjectEntry *ent = new JSonObjectEntry(this, key, child);

    for (pos = begin(); (pos != end() && *ent < *pos); pos++);
    this->insert(pos, ent);
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

bool JSonObject::erase(const std::string &key)
{
    JSonObject::const_iterator it = find(key);
    if (it == this->cend())
        return false;
    delete *it;
    std::list<JSonElement *>::erase(it);
    return true;
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
    JSonObjectEntry *elem = (JSonObjectEntry *) *begin();
    return **elem;
}

const JSonElement *JSonObject::firstChild() const
{
    if (cbegin() == cend())
        return nullptr;
    const JSonObjectEntry *elem = (const JSonObjectEntry *) *cbegin();
    return **elem;
}

std::string JSonObject::stringify() const
{
    return "{ }";
}

JSonObject::DoubleKeyException::DoubleKeyException(unsigned long long pos, const std::string &key, LinearHistory &buf):
    JsonException("Unexpected double key " +key +" for object", pos, buf)
{
}

JSonObject::NotAKeyException::NotAKeyException(unsigned long long pos, LinearHistory &buf):
    JsonException("expected string key for object", pos, buf)
{ }

