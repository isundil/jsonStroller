#include "jsonElement.hh"
#include "jsonContainer.hh"
#include "jsonObjectEntry.hh"

JSonElement::JSonElement(JSonContainer *p): parent(p)
{ }

JSonElement::~JSonElement()
{ }

void JSonElement::setParent(JSonContainer *p)
{
    parent = p;
}

unsigned int JSonElement::getLevel() const
{
    unsigned int level = 0;
    for (const JSonElement *parent = this; parent; parent = parent->parent)
        level++;
    return level;
}

JSonContainer *JSonElement::getParent()
{
    return parent;
}

const JSonContainer *JSonElement::getParent() const
{
    return parent;
}

const JSonElement *JSonElement::findPrev() const
{
    const JSonElement *item = this;
    const JSonContainer *parent = item->getParent();
    if (parent == nullptr)
        return nullptr; // Root node, can't have brothers
    std::list<JSonElement *>::const_iterator it = parent->cbegin();
    const JSonObjectEntry *ent = dynamic_cast<const JSonObjectEntry *>(*it);
    const JSonElement *prevElem = ent ? **ent : (*it);
    if (prevElem == item || (ent && **ent == item))
        return nullptr; // First item
    while ((++it) != parent->cend())
    {
        ent = dynamic_cast<const JSonObjectEntry *>(*it);
        if (*it == item || (ent && **ent == item))
            return prevElem;
        prevElem = ent ? **ent : (*it);
    }
    return nullptr;
}

const JSonElement* JSonElement::findNext() const
{
    const JSonElement *item = this;
    const JSonContainer *parent = item->getParent();
    if (parent == nullptr)
        return nullptr; // Root node, can't have brothers
    JSonContainer::const_iterator it = parent->cbegin();
    while (it != parent->cend())
    {
        const JSonObjectEntry *ent = dynamic_cast<const JSonObjectEntry *>(*it);
        if (*it == item || (ent && **ent == item))
        {
            it++;
            if (it == parent->cend())
                return parent->findNext(); // Last item
            ent = dynamic_cast<const JSonObjectEntry *>(*it);
            if (ent)
                return **ent;
            return *it;
        }
        it++;
    }
    return parent->findNext();
}

