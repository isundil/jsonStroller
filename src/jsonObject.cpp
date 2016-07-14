#include "jsonObject.hh"

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

JSonObject::DoubleKeyException::DoubleKeyException(unsigned long long pos, const std::string &key, WrappedBuffer<char, ERROR_HISTORY_LEN> &buf):
    JsonException("Unexpected double key " +key +" for object", pos, buf)
{
}

JSonObject::NotAKeyException::NotAKeyException(unsigned long long pos, WrappedBuffer<char, ERROR_HISTORY_LEN> &buf):
    JsonException("expected string key for object", pos, buf)
{ }

