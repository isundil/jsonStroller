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

/*
const JSonElement *JSonObjectEntry::findPrev() const
{
    const JSonObject *parent = (JSonObject*) getParent();
    if (parent == nullptr)
        return nullptr; // Root node, can't have brothers
    std::list<JSonElement *>::const_iterator it = parent->cbegin();
    const JSonObjectEntry *ent = (const JSonObjectEntry *)(*it);
    const JSonObjectEntry *prevElem = ent;
    if (prevElem == this)
        return nullptr; // First item
    while ((++it) != parent->cend())
    {
        ent = (const JSonObjectEntry *)(*it);
        if (*it == this)
            return prevElem;
        prevElem = ent;
    }
    return nullptr;
}

const JSonElement* JSonObjectEntry::findNext() const
{
    const JSonObject *parent = (const JSonObject*) this->getParent();
    if (parent == nullptr)
        return nullptr; // Root node, can't have brothers
    JSonContainer::const_iterator it = parent->cbegin();
    while (it != parent->cend())
    {
        if (*it == this)
        {
            it++;
            if (it == parent->cend())
                return parent->findNext(); // Last item
            return *it;
        }
        it++;
    }
    return parent->findNext();
}
*/

