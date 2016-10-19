/**
 * curseOutput.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>

#include "curseSimpleOutput.hh"
#include "searchPattern.hh"
#include "jsonPrimitive.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"

CurseSimpleOutput::CurseSimpleOutput(const Params &p): CurseOutput(p)
{
    init();
}

CurseSimpleOutput::~CurseSimpleOutput()
{
    shutdown();
}

void CurseSimpleOutput::run(JSonElement *root, const std::string &i)
{
    scrollTop = 0;
    selection = data = root;
    inputName = i;
    loop(nullptr);
}

bool CurseSimpleOutput::redraw()
{
    const t_Cursor screenSize = getScreenSize();
    t_Cursor cursor(0, 1);
    /**
     * Will be true if the json's last item is visible
    **/
    bool result;

    select_up = select_down = nullptr;
    selectFound = selectIsLast = false;
    clear();
    writeTopLine(inputName, OutputFlag::SPECIAL_ACTIVEINPUTNAME);
    try {
        result = redraw(cursor, screenSize, data);
    }
    catch (SelectionOutOfRange &e)
    {
        return false;
    }
    if (!result && !selectFound)
    {
        scrollTop++;
        return false;
    }
    if (!result && !select_down)
        selectIsLast = true;
    if (!select_down)
    {
        const JSonContainer *pselect = dynamic_cast<const JSonContainer*>(selection);
        if (pselect && !pselect->empty() && collapsed.find(pselect) == collapsed.cend())
            select_down = *(pselect->cbegin());
        else
        {
            const JSonElement *next = selection->findNext();
            select_down = next ? next : selection;
        }
    }
    if (!select_up)
        select_up = selection;
    refresh();
    return true;
}

inputResult CurseSimpleOutput::selectUp()
{
    selection = select_up;
    return inputResult::redraw;
}

inputResult CurseSimpleOutput::selectDown()
{
    if (selectIsLast)
        scrollTop += 2;
    else if (selection != select_down)
        selection = select_down;
    else
        return inputResult::nextInput;
    return inputResult::redraw;
}

inputResult CurseSimpleOutput::selectPUp()
{
    const JSonElement *_selection = selection;
    const JSonElement *brother = _selection->findPrev();

    if (brother == nullptr)
    {
        const JSonElement *parent = _selection->getParent();
        if (parent && dynamic_cast<const JSonContainer*>(parent))
        {
            selection = _selection = parent;
            if (_selection->getParent() && dynamic_cast<const JSonObjectEntry*> (_selection->getParent()))
                selection = _selection->getParent();
        }
        else
            return inputResult::nextInput;
    }
    else
        selection = brother;
    return inputResult::redraw;
}

inputResult CurseSimpleOutput::selectPDown()
{
    const JSonElement *brother = selection->findNext();

    if (brother)
    {
        selection = brother;
        return inputResult::redraw;
    }
    return inputResult::nextInput;
}

inputResult CurseSimpleOutput::expandSelection()
{
    const JSonElement *_selection = selection;

    if (dynamic_cast<const JSonObjectEntry*>(_selection))
        _selection = **((const JSonObjectEntry*)_selection);
    if (!dynamic_cast<const JSonContainer*>(_selection))
        return inputResult::nextInput;

    if (collapsed.erase((const JSonContainer *)_selection))
        return inputResult::redraw;
    if (!((const JSonContainer*)_selection)->size())
        return inputResult::nextInput;
    selection = select_down;
    return inputResult::redraw;
}

inputResult CurseSimpleOutput::collapseSelection()
{
    const JSonElement *_selection = selection;

    if (dynamic_cast<const JSonObjectEntry*>(_selection))
        _selection = **((const JSonObjectEntry*)_selection);
    if (_selection->getParent() && (!dynamic_cast<const JSonContainer*>(_selection)
            || collapsed.find((const JSonContainer *)_selection) != collapsed.end()
            || (dynamic_cast<const JSonContainer*>(_selection) && ((const JSonContainer*)_selection)->size() == 0)))
    {
        selection = selection->getParent();
        if (selection->getParent() && dynamic_cast<const JSonObjectEntry*>(selection->getParent()))
            selection = selection->getParent();
    }
    else
        collapsed.insert((const JSonContainer *)_selection);
    return inputResult::redraw;
}

inputResult CurseSimpleOutput::initSearch()
{
    const SearchPattern *search_pattern = inputSearch();
    if (!search_pattern)
        return inputResult::redraw;
    search_result.clear();
    if (search_pattern->isEmpty())
        return inputResult::redraw;
    search(*search_pattern, data);
    delete search_pattern;
    return nextResult();
}

inputResult CurseSimpleOutput::nextResult()
{
    if (search_result.empty())
        CurseOutput::redraw("Pattern not found");
    else if (jumpToNextSearch())
        return inputResult::redraw;
    return inputResult::nextInput;
}

inputResult CurseSimpleOutput::changeWindow(char, bool)
{
    //TODO tab mode ?
    return inputResult::nextInput;
}

bool CurseSimpleOutput::redraw(t_Cursor &cursor, const t_Cursor &maxSize, JSonElement *item)
{
    checkSelection(item, cursor);
    if (dynamic_cast<const JSonContainer*>(item))
    {
        if (!writeContainer(cursor, maxSize, (const JSonContainer *) item))
            return false;
    }
    else
    {
        cursor.second += CurseOutput::write(cursor.first, cursor.second, item, maxSize.first, CurseSimpleOutput::getFlag(item));
        if (cursor.second - scrollTop > 0 && (cursor.second - scrollTop) > maxSize.second -1)
            return false;
    }
    return true;
}

bool CurseSimpleOutput::writeContainer(t_Cursor &cursor, const t_Cursor &maxSize, const JSonContainer *item)
{
    char childDelimiter[2];

    if (dynamic_cast<const JSonObject *>(item))
        memcpy(childDelimiter, "{}", sizeof(*childDelimiter) * 2);
    else
        memcpy(childDelimiter, "[]", sizeof(*childDelimiter) * 2);

    if (collapsed.find((const JSonContainer *)item) != collapsed.end())
    {
        std::string ss;
        ss.append(&childDelimiter[0], 1).append(" ... ").append(&childDelimiter[1], 1);
        cursor.second += write(cursor.first, cursor.second, ss, 7, maxSize.first, CurseSimpleOutput::getFlag(item));
    }
    else
    {
        cursor.second += write(cursor.first, cursor.second, childDelimiter[0], maxSize.first, CurseSimpleOutput::getFlag(item));
        if (cursor.second <= scrollTop && cursor.second > maxSize.second +scrollTop -1)
                return false;
        if (!writeContent(cursor, maxSize, (std::list<JSonElement *> *)item))
            return false;
        cursor.second += write(cursor.first, cursor.second, childDelimiter[1], maxSize.first, CurseSimpleOutput::getFlag(item));
    }
    return (cursor.second >= scrollTop || (cursor.second - scrollTop) <= maxSize.second -1);
}

bool CurseSimpleOutput::writeContent(t_Cursor &cursor, const t_Cursor &maxSize, std::list<JSonElement*> *_item)
{
    JSonContainer *item = (JSonContainer *)_item;
    bool containerIsObject = (dynamic_cast<JSonObject *>(item) != nullptr);
    bool result = true;
    cursor.first += INDENT_LEVEL;

    for (JSonElement *i : *item)
    {
        result = false;
        if (containerIsObject)
        {
            JSonObjectEntry *ent = (JSonObjectEntry*) i;
            bool isContainer = (dynamic_cast<JSonContainer *>(**ent) != nullptr);
            std::string key = ent->stringify();
            checkSelection(ent, cursor);
            if (isContainer && collapsed.find((JSonContainer*)(**ent)) != collapsed.cend())
            {
                if (dynamic_cast<JSonObject *>(**ent))
                {
                    if (!CurseOutput::writeKey(key, ent->lazystrlen(), "{ ... }", cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!CurseOutput::writeKey(key, ent->lazystrlen(), "[ ... ]", cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (!isContainer)
            {
                JSonElement *eContent = **ent;
                if (!writeKey(key, ent->lazystrlen(), eContent->stringify(), eContent->lazystrlen(), cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (((JSonContainer*)(**ent))->size() == 0)
            {
                if (dynamic_cast<const JSonObject *>(**ent) )
                {
                    if (!CurseOutput::writeKey(key, ent->lazystrlen(), "{ }", cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!CurseOutput::writeKey(key, ent->lazystrlen(), "[ ]", cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else
            {
                if (!writeKey(key, ent->lazystrlen(), cursor, maxSize, CurseSimpleOutput::getFlag(ent)))
                    break;
                const JSonElement *saveSelection = selection;
                if (selection == ent)
                    selection = **ent;
                cursor.first += INDENT_LEVEL /2;
                if (!redraw(cursor, maxSize, **ent))
                {
                    selection = saveSelection;
                    cursor.first -= INDENT_LEVEL /2;
                    return false;
                }
                selection = saveSelection;
                cursor.first -= INDENT_LEVEL /2;
            }
        }
        else
        {
            if (!redraw(cursor, maxSize, i))
                break;
        }
        result = true;
    }
    cursor.first -= INDENT_LEVEL;
    //result will be false if for loop break'd at some time, true otherwise
    return result;
}

bool CurseSimpleOutput::writeKey(const std::string &key, const size_t keylen, t_Cursor &cursor, const t_Cursor &maxSize, OutputFlag flags, unsigned int extraLen)
{
    if (cursor.second - scrollTop <= 0)
    {
        cursor.second++;
        return true;
    }
    char oldType = flags.type();
    flags.type(OutputFlag::TYPE_OBJKEY);
    cursor.second += write(cursor.first, cursor.second, key, keylen, maxSize.first -extraLen -2, flags);
    flags.type(OutputFlag::TYPE_OBJ);
    write(": ", flags);
    flags.type(oldType);
    return (cursor.second >= scrollTop || cursor.second <= maxSize.second +scrollTop);
}

bool CurseSimpleOutput::writeKey(const std::string &key, const size_t keylen, const std::string &after, size_t afterlen, t_Cursor &cursor, const t_Cursor &maxSize, OutputFlag flags)
{
    if (cursor.second - scrollTop <= 0)
    {
        cursor.second++;
        return true;
    }
    char oldType = flags.type();
    flags.type(OutputFlag::TYPE_OBJKEY);
    write(cursor.first, cursor.second, key, 0, 1, flags);
    flags.type(OutputFlag::TYPE_OBJ);
    write(": ", flags);
    flags.type(oldType);
    write(after.c_str(), flags);
    cursor.second += getNbLines(cursor.first +keylen +2 +afterlen, maxSize.first);
    return (cursor.second >= scrollTop || (cursor.second - scrollTop) <= maxSize.second);
}

unsigned int CurseSimpleOutput::write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags)
{
    int offsetY = y - scrollTop;
    char color = OutputFlag::SPECIAL_NONE;

    if (offsetY <= 0)
        return 1;

    if (flags.selected())
        attron(A_REVERSE | A_BOLD);
    if (flags.searched())
        color = OutputFlag::SPECIAL_SEARCH;
    else if (colors.find(flags.type()) != colors.end())
        color = flags.type();

    if (color != OutputFlag::SPECIAL_NONE)
        attron(COLOR_PAIR(color));
    mvprintw(offsetY, x, "%c", item);
    attroff(A_REVERSE | A_BOLD);
    if (color != OutputFlag::SPECIAL_NONE)
        attroff(COLOR_PAIR(color));
    return getNbLines(x +1, maxWidth);
}

unsigned int CurseSimpleOutput::write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags)
{
    int offsetY = y - scrollTop;
    if (offsetY <= 0)
        return 1;
    move(offsetY, x);
    write(str, flags);
    return getNbLines(strlen +x, maxWidth);
}

void CurseSimpleOutput::write(const std::string &str, const OutputFlag flags) const
{
    char color = OutputFlag::SPECIAL_NONE;
    if (flags.selected())
        attron(A_REVERSE | A_BOLD);
    if (flags.searched())
        color = OutputFlag::SPECIAL_SEARCH;
    else if (colors.find(flags.type()) != colors.end())
        color = flags.type();
    if (color != OutputFlag::SPECIAL_NONE)
        attron(COLOR_PAIR(color));

    printw("%s", str.c_str());
    attroff(A_REVERSE | A_BOLD);
    if (color != OutputFlag::SPECIAL_NONE)
        attroff(COLOR_PAIR(color));
}

bool CurseSimpleOutput::jumpToNextSearch(const JSonElement *current, bool &selectFound)
{
    const JSonContainer *container = dynamic_cast<const JSonContainer *> (current);
    const JSonObjectEntry *objEntry = dynamic_cast<const JSonObjectEntry *> (current);

    if (selection == current)
        selectFound = true;
    if (container)
    {
        if (!container->empty())
            for (const JSonElement *it : *container)
                if (jumpToNextSearch(it, selectFound))
                    return true;
    }
    else
    {
        if (current && std::find(search_result.cbegin(), search_result.cend(), current) != search_result.cend() && current != selection && selectFound)
        {
            selection = current;
            return true;
        }
        if (objEntry)
            if (jumpToNextSearch(**objEntry, selectFound))
                return true;
    }
    return false;
}

const OutputFlag CurseSimpleOutput::getFlag(const JSonElement *e) const
{
    return getFlag(e, selection);
}

const OutputFlag CurseSimpleOutput::getFlag(const JSonElement *item, const JSonElement *selection) const
{
    OutputFlag res;
    const JSonElement *i = dynamic_cast<const JSonObjectEntry*>(item) ? **((const JSonObjectEntry*)item) : item;

    res.selected(item == selection);
    res.searched(std::find(search_result.cbegin(), search_result.cend(), item) != search_result.cend());
    if (dynamic_cast<const JSonPrimitive<std::string> *>(i))
        res.type(OutputFlag::TYPE_STRING);
    else if (dynamic_cast<const JSonPrimitive<bool> *>(i))
        res.type(OutputFlag::TYPE_BOOL);
    else if (dynamic_cast<const JSonPrimitive<Null> *>(i))
        res.type(OutputFlag::TYPE_NULL);
    else if (dynamic_cast<const AJSonPrimitive *>(i))
        res.type(OutputFlag::TYPE_NUMBER);
    else if (dynamic_cast<const JSonObject*>(i))
        res.type(OutputFlag::TYPE_OBJ);
    else if (dynamic_cast<const JSonArray*>(i))
        res.type(OutputFlag::TYPE_ARR);
    return res;
}

unsigned int CurseSimpleOutput::search(const SearchPattern &search_pattern, const JSonElement *current)
{
    const JSonContainer *container = dynamic_cast<const JSonContainer *> (current);
    const JSonObjectEntry *objEntry = dynamic_cast<const JSonObjectEntry *> (current);
    unsigned int result =0;

    if (container)
    {
        if (!container->empty())
            for (const JSonElement *it : *container)
                result += search(search_pattern, it);
    }
    else
    {
        if (current && current->match(search_pattern))
        {
            if (current->getParent() && dynamic_cast<const JSonObjectEntry*>(current->getParent()))
                search_result.push_back(current->getParent());
            else
                search_result.push_back(current);
            result++;
        }
        if (objEntry)
            result += search(search_pattern, **objEntry);
    }
    result = search_result.size();
    return result;
}

bool CurseSimpleOutput::jumpToNextSearch()
{
    bool selectFound = false;
    bool res = jumpToNextSearch(data, selectFound);

    if (!res)
    {
        selection = *(search_result.cbegin());
        unfold(selection);
        CurseOutput::redraw("Search hit BOTTOM, continuing at TOP");
        return false;
    }
    unfold(selection);
    return true;
}

void CurseSimpleOutput::checkSelection(const JSonElement *item, const t_Cursor &cursor)
{
    if (!selectFound)
    {
        if (selection == item)
        {
            if (cursor.second < scrollTop) //Selection is above vp, move scroll pos to selection and start drawing
                scrollTop = cursor.second;
            selectFound = true;
        }
        else if (!item->getParent() || !dynamic_cast<const JSonObjectEntry*>(item->getParent()))
            select_up = item;
    }
    else if (!select_down)
    {
        const JSonElement *parent = item->getParent();
        if (!dynamic_cast<const JSonContainer*>(item) && parent && selection != parent && dynamic_cast<const JSonObjectEntry*>(parent))
            item = parent;
        if (!parent || !dynamic_cast<const JSonObjectEntry*>(parent))
            select_down = item;
    }
}

void CurseSimpleOutput::shutdown()
{
    endwin();
    delscreen(screen);
    if (screen_fd)
    {
        fclose(screen_fd);
        screen_fd = nullptr;
    }
    screen = nullptr;
}


