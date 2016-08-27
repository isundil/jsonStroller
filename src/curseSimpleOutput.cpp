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

void CurseSimpleOutput::run(JSonElement *root)
{
    selection = data = root;
    loop();
}

bool CurseSimpleOutput::redraw()
{
    const std::pair<unsigned int, unsigned int> screenSize = getScreenSize();
    std::pair<int, int> cursor(0, 0);
    /**
     * Will be true if the json's last item is visible
    **/
    bool result;

    select_up = select_down = nullptr;
    selectFound = selectIsLast = false;
    clear();
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
        if (pselect && !pselect->empty())
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

Optional<bool> CurseSimpleOutput::evalKey(int c)
{
    switch (c)
    {
        case 'q':
        case 'Q':
            return Optional<bool>::of(false);

        case KEY_UP:
        case 'K':
        case 'k':
            selection = select_up;
            return Optional<bool>::of(true);

        case KEY_DOWN:
        case 'j':
        case 'J':
            if (selectIsLast)
                scrollTop += 2;
            else if (selection != select_down)
                selection = select_down;
            else
                break;
            return Optional<bool>::of(true);

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
            return Optional<bool>::of(true);
        }

        case KEY_NPAGE:
        {
            const JSonElement *brother = selection->findNext();

            if (brother)
            {
                selection = brother;
                return Optional<bool>::of(true);
            }
            break;
        }

        case 'l':
        case 'L':
        case KEY_RIGHT:
        {
            if (dynamic_cast<const JSonObjectEntry*>(selection))
                selection = **((const JSonObjectEntry*)selection);
            if (!dynamic_cast<const JSonContainer*>(selection))
                return Optional<bool>::empty;

            if (collapsed.erase((const JSonContainer *)selection))
                return Optional<bool>::of(true);
            if (!((const JSonContainer*)selection)->size())
                break;
            selection = select_down;
            return Optional<bool>::of(true);
        }

        case 'h':
        case 'H':
        case KEY_LEFT:
        {
            if (dynamic_cast<const JSonObjectEntry*>(selection))
                selection = **((const JSonObjectEntry*)selection);
            if (selection->getParent() && (!dynamic_cast<const JSonContainer*>(selection)
                    || collapsed.find((const JSonContainer *)selection) != collapsed.end()
                    || (dynamic_cast<const JSonContainer*>(selection) && ((const JSonContainer*)selection)->size() == 0)))
            {
                selection = selection->getParent();
                if (selection->getParent() && dynamic_cast<const JSonObjectEntry*>(selection->getParent()))
                    selection = selection->getParent();
            }
            else if (selection)
                collapsed.insert((const JSonContainer *)selection);
            else
                break;
            return Optional<bool>::of(false);
        }

        case '/':
        {
            const SearchPattern *search_pattern = inputSearch();
            if (!search_pattern)
                return Optional<bool>::of(true);
            search_result.clear();
            if (search_pattern->isEmpty())
                return Optional<bool>::of(true);
            search(*search_pattern, data);
            delete search_pattern;
        }

        case 'n':
        case 'N':
            if (search_result.empty())
                CurseOutput::redraw("Pattern not found");
            else if (jumpToNextSearch())
                return Optional<bool>::of(true);
            break;
    }
    return Optional<bool>::empty;
}

bool CurseSimpleOutput::redraw(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, JSonElement *item)
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
        if (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1)
            return false;
    }
    return true;
}

bool CurseSimpleOutput::writeContainer(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *item)
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
        if (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1)
                return false;
        if (!writeContent(cursor, maxSize, (std::list<JSonElement *> *)item))
            return false;
        cursor.second += write(cursor.first, cursor.second, childDelimiter[1], maxSize.first, CurseSimpleOutput::getFlag(item));
    }
    return (cursor.second - scrollTop < 0 || (unsigned)(cursor.second - scrollTop) <= maxSize.second -1);
}

bool CurseSimpleOutput::writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, std::list<JSonElement*> *_item)
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
                    if (!CurseOutput::writeKey(key, ent->lazystrlen(), "{ ... }", cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!CurseOutput::writeKey(key, ent->lazystrlen(), "[ ... ]", cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (!isContainer)
            {
                JSonElement *eContent = **ent;
                if (!writeKey(key, ent->lazystrlen(), eContent->stringify(), eContent->lazystrlen(), cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (((JSonContainer*)(**ent))->size() == 0)
            {
                if (dynamic_cast<const JSonObject *>(**ent) )
                {
                    if (!CurseOutput::writeKey(key, ent->lazystrlen(), "{ }", cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!CurseOutput::writeKey(key, ent->lazystrlen(), "[ ]", cursor, maxSize, CurseSimpleOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
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

bool CurseSimpleOutput::writeKey(const std::string &key, const size_t keylen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags, unsigned int extraLen)
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

bool CurseSimpleOutput::writeKey(const std::string &key, const size_t keylen, const std::string &after, size_t afterlen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags)
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

unsigned int CurseSimpleOutput::write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags)
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

unsigned int CurseSimpleOutput::write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags)
{
    int offsetY = y - scrollTop;
    if (offsetY < 0)
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

void CurseSimpleOutput::checkSelection(const JSonElement *item, const std::pair<int, int> &cursor)
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

void CurseSimpleOutput::init()
{
    if (!isatty(fileno(stdin)) || !isatty(fileno(stdout)))
    {
        screen_fd = fopen("/dev/tty", "r+");
        setbuf(screen_fd, nullptr);
        screen = newterm(nullptr, screen_fd, screen_fd);
    }
    else
    {
        screen = newterm(nullptr, stdout, stdin);
        screen_fd = nullptr;
    }
    wtimeout(stdscr, 150);
    cbreak();
    clear();
    noecho();
    curs_set(false);
    keypad(stdscr, true);

    if (params.colorEnabled())
    {
        start_color();
        init_pair(OutputFlag::TYPE_NUMBER, COLOR_GREEN, COLOR_BLACK);
        init_pair(OutputFlag::TYPE_BOOL, COLOR_RED, COLOR_BLACK);
        init_pair(OutputFlag::TYPE_NULL, COLOR_RED, COLOR_BLACK);
        init_pair(OutputFlag::TYPE_STRING, COLOR_CYAN, COLOR_BLACK);
        init_pair(OutputFlag::TYPE_OBJKEY, COLOR_CYAN, COLOR_BLACK);
        init_pair(OutputFlag::SPECIAL_SEARCH, COLOR_WHITE, COLOR_BLUE);
        init_pair(OutputFlag::SPECIAL_ERROR, COLOR_WHITE, COLOR_RED);
        colors.insert(OutputFlag::TYPE_NUMBER);
        colors.insert(OutputFlag::TYPE_BOOL);
        colors.insert(OutputFlag::TYPE_STRING);
        colors.insert(OutputFlag::TYPE_OBJKEY);
        colors.insert(OutputFlag::TYPE_NULL);
    }

    signal(SIGWINCH, _resizeFnc);
    signal(SIGINT, _resizeFnc);
    signal(SIGTERM, _resizeFnc);
    signal(SIGKILL, _resizeFnc);
    scrollTop = 0;
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

