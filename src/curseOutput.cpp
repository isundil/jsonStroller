/**
 * curseOutput.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <iostream>

#include <sys/ioctl.h>
#include <algorithm>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "searchPattern.hh"
#include "curseOutput.hh"
#include "jsonPrimitive.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "except.hh"
#include "streamConsumer.hh"

static CurseOutput *runningInst = nullptr;

CurseOutput::CurseOutput(const Params &p): data(nullptr), params(p)
{
    runningInst = this;
}

CurseOutput::~CurseOutput()
{
    runningInst = nullptr;
}

void CurseOutput::loop()
{
    breakLoop = false;
    do
    {
        while (!redraw()) //TODO opti going down
            ;
    } while(readInput());
}

bool CurseOutput::onsig(int signo)
{
    struct winsize size;

    switch (signo)
    {
    case SIGWINCH:
        if (ioctl(fileno(screen_fd ? screen_fd : stdout), TIOCGWINSZ, &size) == 0)
            resize_term(size.ws_row, size.ws_col);
        clear();
        while (!redraw());
        break;

    case SIGKILL:
    case SIGINT:
    case SIGTERM:
        breakLoop = true;
        break;

    default:
        return false;
    }
    return true;
}

void _resizeFnc(int signo)
{
    if (!runningInst)
        return;
    runningInst->onsig(signo);
}

bool CurseOutput::redraw(const std::string &errorMsg)
{
    bool result = redraw();
    writeBottomLine(errorMsg, OutputFlag::SPECIAL_ERROR);
    return result;
}

bool CurseOutput::redraw(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, JSonElement *item)
{
    checkSelection(item, cursor);
    if (dynamic_cast<const JSonContainer*>(item))
    {
        if (!writeContainer(cursor, maxSize, (const JSonContainer *) item))
            return false;
    }
    else
    {
        cursor.second += write(cursor.first, cursor.second, item, maxSize.first, getFlag(item));
        if (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1)
            return false;
    }
    return true;
}

bool CurseOutput::writeContainer(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *item)
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
        cursor.second += write(cursor.first, cursor.second, ss, 7, maxSize.first, getFlag(item));
    }
    else
    {
        cursor.second += write(cursor.first, cursor.second, childDelimiter[0], maxSize.first, getFlag(item));
        if (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1)
                return false;
        if (!writeContent(cursor, maxSize, (std::list<JSonElement *> *)item))
            return false;
        cursor.second += write(cursor.first, cursor.second, childDelimiter[1], maxSize.first, getFlag(item));
    }
    return (cursor.second - scrollTop < 0 || (unsigned)(cursor.second - scrollTop) <= maxSize.second -1);
}

bool CurseOutput::writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, std::list<JSonElement*> *_item)
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
                    if (!writeKey(key, ent->lazystrlen(), "{ ... }", cursor, maxSize, getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!writeKey(key, ent->lazystrlen(), "[ ... ]", cursor, maxSize, getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (!isContainer)
            {
                JSonElement *eContent = **ent;
                if (!writeKey(key, ent->lazystrlen(), eContent->stringify(), eContent->lazystrlen(), cursor, maxSize, getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (((JSonContainer*)(**ent))->size() == 0)
            {
                if (dynamic_cast<const JSonObject *>(**ent) )
                {
                    if (!writeKey(key, ent->lazystrlen(), "{ }", cursor, maxSize, getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!writeKey(key, ent->lazystrlen(), "[ ]", cursor, maxSize, getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else
            {
                if (!writeKey(key, ent->lazystrlen(), cursor, maxSize, getFlag(ent)))
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

bool CurseOutput::writeKey(const std::string &key, const size_t keylen, const std::string &after, size_t afterlen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags)
{
    if (cursor.second - scrollTop < 0)
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
    return (cursor.second - scrollTop < 0 || (unsigned)(cursor.second - scrollTop) <= maxSize.second);
}

bool CurseOutput::writeKey(const std::string &key, const size_t keylen, const std::string &after, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags)
{
    return writeKey(key, keylen, after, after.size(), cursor, maxSize, flags);
}

bool CurseOutput::writeKey(const std::string &key, const size_t keylen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags, unsigned int extraLen)
{
    if (cursor.second - scrollTop < 0)
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
    return (cursor.second - scrollTop < 0 || (unsigned)(cursor.second - scrollTop) <= maxSize.second);
}

unsigned int CurseOutput::write(const int &x, const int &y, JSonElement *item, unsigned int maxWidth, OutputFlag flags)
{
    return write(x, y, item->stringify(), item->lazystrlen(), maxWidth, flags);
}

unsigned int CurseOutput::write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags)
{
    int offsetY = y - scrollTop;
    char color = OutputFlag::SPECIAL_NONE;

    if (offsetY < 0)
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

void CurseOutput::write(const std::string &str, const OutputFlag flags) const
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

unsigned int CurseOutput::write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags)
{
    int offsetY = y - scrollTop;
    if (offsetY < 0)
        return 1;
    move(offsetY, x);
    write(str, flags);
    return getNbLines(strlen +x, maxWidth);
}

unsigned int CurseOutput::getNbLines(const size_t nbChar, unsigned int maxWidth)
{
    double nLine = (double) nbChar / maxWidth;
    if (nLine == (unsigned int) nLine)
        return nLine;
    return nLine +1;
}

const std::pair<unsigned int, unsigned int> CurseOutput::getScreenSize() const
{
    std::pair<int, int> bs;
    std::pair<int, int> sc;
    getmaxyx(stdscr, sc.second, sc.first);
    getbegyx(stdscr, bs.second, bs.first);
    sc.first -= bs.first;
    sc.second -= bs.second;
    return sc;
}

const OutputFlag CurseOutput::getFlag(const JSonElement *item) const
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

void CurseOutput::checkSelection(const JSonElement *item, const std::pair<int, int> &cursor)
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

/**
 * Read input and expect signal
 * @Return true on:
 *  - Windows resized
 *  - Key press and need redraw
 * false on:
 *  - exit signal
**/
bool CurseOutput::readInput()
{
    while (!breakLoop)
    {
        int c;
        c = getch();

        switch (c)
        {
            case 'q':
            case 'Q':
                return false;

            case KEY_UP:
            case 'K':
            case 'k':
                selection = select_up;
                return true;

            case KEY_DOWN:
            case 'j':
            case 'J':
                if (selectIsLast)
                    scrollTop += 2;
                else if (selection != select_down)
                    selection = select_down;
                else
                    break;
                return true;

            case KEY_PPAGE:
            {
                const JSonElement *brother = selection->findPrev();
                if (brother == nullptr)
                {
                    const JSonElement *parent = selection->getParent();
                    if (parent && dynamic_cast<const JSonContainer*>(parent))
                    {
                        selection = parent;
                        if (selection->getParent() && dynamic_cast<const JSonObjectEntry*> (selection->getParent()))
                            selection = selection->getParent();
                    }
                    else
                        break;
                }
                else
                    selection = brother;
                return true;
                break;
            }

            case KEY_NPAGE:
            {
                const JSonElement *brother = selection->findNext();
                if (brother)
                {
                    selection = brother;
                    return true;
                }
                break;
            }

            case 'l':
            case 'L':
            case KEY_RIGHT:
            {
                const JSonElement *_selection = selection;
                if (dynamic_cast<const JSonObjectEntry*>(selection))
                    _selection = **((const JSonObjectEntry*)_selection);
                if (!dynamic_cast<const JSonContainer*>(_selection))
                    break;

                if (collapsed.erase((const JSonContainer *)_selection))
                    return true;
                if (!((const JSonContainer*)_selection)->size())
                    break;
                selection = select_down;
                return true;
            }

            case 'h':
            case 'H':
            case KEY_LEFT:
            {
                const JSonElement *_selection = selection;
                if (dynamic_cast<const JSonObjectEntry*>(_selection))
                    _selection = **((const JSonObjectEntry*)_selection);
                if (selection->getParent() && (!dynamic_cast<const JSonContainer*>(_selection)
                        || collapsed.find((const JSonContainer *)_selection) != collapsed.end()
                        || (dynamic_cast<const JSonContainer*>(_selection) && ((const JSonContainer*)_selection)->size() == 0)))
                {
                    selection = selection->getParent();
                    if (selection->getParent() && dynamic_cast<const JSonObjectEntry*>(selection->getParent()))
                        selection = selection->getParent();
                }
                else if (_selection)
                    collapsed.insert((const JSonContainer *)_selection);
                else
                    break;
                return true;
            }

            case '/':
            {
                const SearchPattern *search_pattern = inputSearch();
                if (!search_pattern)
                    return true;
                search_result.clear();
                if (search_pattern->isEmpty())
                    return true;
                search(*search_pattern, data);
                delete search_pattern;
            }

            case 'n':
            case 'N':
                if (search_result.empty())
                    this->redraw("Pattern not found");
                else if (jumpToNextSearch())
                    return true;
                break;
        }
    }
    return false;
}

unsigned int CurseOutput::search(const SearchPattern &search_pattern, const JSonElement *current)
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
            {
                if (current->getParent() != selection)
                    search_result.push_back(current->getParent());
            }
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

bool CurseOutput::jumpToNextSearch(const JSonElement *current, bool &selectFound)
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

bool CurseOutput::jumpToNextSearch()
{
    bool selectFound = false;
    bool res = jumpToNextSearch(data, selectFound);

    if (!res)
    {
        selection = *(search_result.cbegin());
        unfold(selection);
        redraw("Search hit BOTTOM, continuing at TOP");
        return false;
    }
    unfold(selection);
    return true;
}

void CurseOutput::unfold(const JSonElement *item)
{
    while (item->getParent())
    {
        collapsed.erase((const JSonContainer*)item->getParent());
        item = item->getParent();
    }
}

const SearchPattern *CurseOutput::inputSearch()
{
    std::wstring buffer;
    bool abort = false;

    curs_set(true);
    wtimeout(stdscr, -1);
    while (!abort)
    {
        int c;

        writeBottomLine(L'/' +buffer, OutputFlag::SPECIAL_SEARCH);
        refresh();
        c = getwchar();
        if (c == L'\r')
            break;
        else if (c == L'\b' || c == 127)
        {
            if (!buffer.empty())
                buffer.pop_back();
        }
        else if (c == 27)
            abort = true;
        else
            buffer += c;
    }
    wtimeout(stdscr, 150);
    curs_set(false);

    {
        const size_t size = buffer.size();
        char bytesString[size * sizeof(wchar_t)];
        wcstombs(&bytesString[0], buffer.c_str(), sizeof(bytesString));
        std::string str;
        if (params.isIgnoringUnicode())
            str = bytesString;
        else
            str = StreamConsumer::extractUnicode(bytesString);
        return abort ? nullptr : new SearchPattern(str);
    }
}

void CurseOutput::writeBottomLine(const std::string &buffer, short color) const
{
    const std::pair<unsigned int, unsigned int> screenSize = getScreenSize();
    const size_t bufsize = buffer.size();

    if (params.colorEnabled())
        attron(COLOR_PAIR(color));
    mvprintw(screenSize.second -1, 0, "%s%*c", buffer.c_str(), screenSize.first - bufsize, ' ');
    move(screenSize.second -1, bufsize);
    if (params.colorEnabled())
        attroff(COLOR_PAIR(color));
}

void CurseOutput::writeBottomLine(const std::wstring &buffer, short color) const
{
    const std::pair<unsigned int, unsigned int> screenSize = getScreenSize();
    const size_t bufsize = buffer.size();

    if (params.colorEnabled())
        attron(COLOR_PAIR(color));
    mvprintw(screenSize.second -1, 0, "%S%*c", buffer.c_str(), screenSize.first - bufsize, ' ');
    move(screenSize.second -1, bufsize);
    if (params.colorEnabled())
        attroff(COLOR_PAIR(color));
}

