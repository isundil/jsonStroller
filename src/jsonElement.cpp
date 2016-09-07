/**
 * jsonElement.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "jsonElement.hh"
#include "jsonContainer.hh"
#include "jsonObjectEntry.hh"
#include "searchPattern.hh"
#include "levenshtein.hh"

JSonElement::JSonElement(JSonElement *p): parent(p), _strlen(0)
{ }

JSonElement::~JSonElement()
{ }

void JSonElement::setParent(JSonElement *p)
{
    parent = p;
}

size_t JSonElement::lazystrlen()
{
    if (_strlen)
        return _strlen;
    const std::string buf = stringify();
    if (!buf.size())
        return _strlen;
    wchar_t wbuf[buf.size()];
    mbstowcs(wbuf, buf.c_str(), buf.size() * sizeof(wchar_t));
    return _strlen = wcslen(wbuf);
}

size_t JSonElement::strlen() const
{
    if (_strlen)
        return _strlen;
    const std::string buf = stringify();
    if (!buf.size())
        return _strlen;
    wchar_t wbuf[buf.size()];
    mbtowc(wbuf, buf.c_str(), buf.size());
    return wcslen(wbuf);
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

bool JSonElement::match(const SearchPattern &searchPattern) const
{
    return searchPattern.match(stringify(), this);
}

float JSonElement::diff(const JSonElement &o) const
{
    return levenshteinPercent(stringify(), o.stringify());
}

