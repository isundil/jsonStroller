#include "jsonElement.hh"
#include "jsonContainer.hh"
#include "jsonObjectEntry.hh"

JSonElement::JSonElement(JSonElement *p): parent(p)
{ }

JSonElement::~JSonElement()
{ }

void JSonElement::setParent(JSonElement *p)
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

JSonElement *JSonElement::getParent()
{
    return parent;
}

const JSonElement *JSonElement::getParent() const
{
    return parent;
}

const JSonElement *JSonElement::findPrev() const
{
    const JSonElement *item = this;
    const JSonElement *parent = item->getParent();
    if (parent == nullptr || !dynamic_cast<const JSonContainer*>(parent))
        return nullptr; // Root node, can't have brothers
    std::list<JSonElement *>::const_iterator it = ((const JSonContainer*)parent)->cbegin();
    const JSonElement *prevElem = *it;
    if (prevElem == item)
        return nullptr; // First item
    while ((++it) != ((const JSonContainer*)parent)->cend())
    {
        if (*it == item)
            return prevElem;
        prevElem = *it;
    }
    return nullptr;
}

const JSonElement* JSonElement::findNext() const
{
    const JSonElement *item = this;
    const JSonElement *parent = item->getParent();
    if (parent == nullptr || !dynamic_cast<const JSonContainer*>(parent))
        return nullptr; // Root node, can't have brothers
    JSonContainer::const_iterator it = ((const JSonContainer*)parent)->cbegin();
    while (it != ((const JSonContainer*)parent)->cend())
    {
        if (*it == item)
        {
            it++;
            if (it == ((const JSonContainer*)parent)->cend())
                return parent->findNext(); // Last item
            return *it;
        }
        it++;
    }
    return parent->findNext();
}

bool JSonElement::match(const std::string &search_pattern) const
{
    const std::string str = stringify();
    return str.find(search_pattern) != str.npos;
}
