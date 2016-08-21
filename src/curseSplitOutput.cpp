/**
 * curseSplitOutput.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <curses.h>

#include "searchPattern.hh"
#include "curseSplitOutput.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"

CurseSplitOutput::CurseSplitOutput(const Params &p): CurseOutput(p)
{
    init();
}

CurseSplitOutput::~CurseSplitOutput()
{
    shutdown();
}

void CurseSplitOutput::run(const std::deque<std::string> &inputName, const std::deque<JSonElement*> &roots)
{
    nbInputs = inputName.size();
    selectedWin = 0;
    scrollTop.clear();
    select_up.clear();
    select_down.clear();
    selection.clear();
    for (size_t i =0; i < nbInputs; i++)
    {
        this->roots.push_back(roots.at(i));
        scrollTop.push_back(0);
        selection.push_back(roots.at(i));
        select_up.push_back(nullptr);
        select_down.push_back(nullptr);
    }
    fileNames = inputName;
    loop();
}

Optional<bool> CurseSplitOutput::evalKey(int c)
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
                scrollTop[selectedWin] += 2;
            else if (selection != select_down)
                selection = select_down;
            else
                break;
            return Optional<bool>::of(true);

        case KEY_PPAGE:
        {
            const JSonElement *_selection = selection[selectedWin];
            const JSonElement *brother = _selection->findPrev();

            if (brother == nullptr)
            {
                const JSonElement *parent = _selection->getParent();
                if (parent && dynamic_cast<const JSonContainer*>(parent))
                {
                    selection[selectedWin] = _selection = parent;
                    if (_selection->getParent() && dynamic_cast<const JSonObjectEntry*> (_selection->getParent()))
                        selection[selectedWin] = _selection->getParent();
                }
                else
                    break;
            }
            else
                selection[selectedWin] = brother;
            return Optional<bool>::of(true);
        }

        case KEY_NPAGE:
        {
            const JSonElement *brother = selection[selectedWin]->findNext();

            if (brother)
            {
                selection[selectedWin] = brother;
                return Optional<bool>::of(true);
            }
            break;
        }

        case 'l':
        case 'L':
        case KEY_RIGHT:
        {
            const JSonElement *_selection = selection[selectedWin];
            if (dynamic_cast<const JSonObjectEntry*>(selection[selectedWin]))
                _selection = **((const JSonObjectEntry*)_selection);
            if (!dynamic_cast<const JSonContainer*>(_selection))
                return Optional<bool>::empty;

            if (collapsed.erase((const JSonContainer *)_selection))
                return Optional<bool>::of(true);
            if (!((const JSonContainer*)_selection)->size())
                break;
            selection[selectedWin] = select_down[selectedWin];
            return Optional<bool>::of(true);
        }

        case 'h':
        case 'H':
        case KEY_LEFT:
        {
            const JSonElement *_selection = selection[selectedWin];
            if (dynamic_cast<const JSonObjectEntry*>(_selection))
                _selection = **((const JSonObjectEntry*)_selection);
            if (_selection->getParent() && (!dynamic_cast<const JSonContainer*>(_selection)
                    || collapsed.find((const JSonContainer *)_selection) != collapsed.end()
                    || (dynamic_cast<const JSonContainer*>(_selection) && ((const JSonContainer*)_selection)->size() == 0)))
            {
                selection[selectedWin] = _selection = _selection->getParent();
                if (_selection->getParent() && dynamic_cast<const JSonObjectEntry*>(_selection->getParent()))
                    selection[selectedWin] = _selection->getParent();
            }
            else if (_selection)
                collapsed.insert((const JSonContainer *)_selection);
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
            search(*search_pattern);
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

void CurseSplitOutput::checkSelection(const JSonElement *item, const std::pair<int, int> &cursor)
{
    if (!selectFound)
    {
        if (selection[workingWin] == item)
        {
            if (cursor.second < scrollTop[workingWin]) //Selection is above vp, move scroll pos to selection and start drawing
                scrollTop[workingWin] = cursor.second;
            selectFound = true;
        }
        else if (!item->getParent() || !dynamic_cast<const JSonObjectEntry*>(item->getParent()))
            select_up[workingWin] = item;
    }
    else if (!select_down[workingWin])
    {
        const JSonElement *parent = item->getParent();
        if (!dynamic_cast<const JSonContainer*>(item) && parent && selection[workingWin] != parent && dynamic_cast<const JSonObjectEntry*>(parent))
            item = parent;
        if (!parent || !dynamic_cast<const JSonObjectEntry*>(parent))
            select_down[workingWin] = item;
    }
}

bool CurseSplitOutput::jumpToNextSearch(const JSonElement *current, bool &selectFound)
{
    const JSonContainer *container = dynamic_cast<const JSonContainer *> (current);
    const JSonObjectEntry *objEntry = dynamic_cast<const JSonObjectEntry *> (current);

    if (selection[selectedWin] == current)
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
        if (current && std::find(search_result[selectedWin].cbegin(), search_result[selectedWin].cend(), current) != search_result[selectedWin].cend() && current != selection[selectedWin] && selectFound)
        {
            selection[selectedWin] = current;
            return true;
        }
        if (objEntry)
            if (jumpToNextSearch(**objEntry, selectFound))
                return true;
    }
    return false;
}

unsigned int CurseSplitOutput::search(const SearchPattern &search_pattern)
{
    unsigned int result =0;

    for (workingWin =0; workingWin < nbInputs; ++workingWin)
        result += search(search_pattern, roots[workingWin]);
    return result;
}

unsigned int CurseSplitOutput::search(const SearchPattern &search_pattern, const JSonElement *current)
{
    const JSonContainer *container = dynamic_cast<const JSonContainer *> (current);
    const JSonObjectEntry *objEntry = dynamic_cast<const JSonObjectEntry *> (current);

    if (container)
    {
        if (!container->empty())
            for (const JSonElement *it : *container)
                search(search_pattern, it);
    }
    else
    {
        if (current && current->match(search_pattern))
        {
            if (current->getParent() && dynamic_cast<const JSonObjectEntry*>(current->getParent()))
                search_result[workingWin].push_back(current->getParent());
            else
                search_result[workingWin].push_back(current);
        }
        if (objEntry)
            search(search_pattern, **objEntry);
    }
    return search_result.size();
}

bool CurseSplitOutput::jumpToNextSearch()
{
    bool selectFound = false;
    bool res = jumpToNextSearch(roots[selectedWin], selectFound);

    if (!res)
    {
        selection[selectedWin] = *(search_result[selectedWin].cbegin());
        unfold(selection[selectedWin]);
        CurseOutput::redraw("Search hit BOTTOM, continuing at TOP");
        return false;
    }
    unfold(selection[selectedWin]);
    return true;
}

bool CurseSplitOutput::redraw()
{
    const std::pair<unsigned int, unsigned int> screenSize = getScreenSize();

    destroyAllSubWin();
    clear();
    for (workingWin =0; workingWin < nbInputs; workingWin++)
    {
        std::pair<int, int> cursor(workingWin * screenSize.first + (workingWin ? 0 : 1), 0);
        bool result;

        currentWin = newwin(screenSize.second, screenSize.first, 0, cursor.first -1);
        box(currentWin, 0, 0);
        select_up[workingWin] = select_down[workingWin] = nullptr;
        selectFound = selectIsLast = false;

        try {
            result = redraw(cursor, screenSize, roots[workingWin]);
        }
        catch (SelectionOutOfRange &e)
        {
            return false;
        }
        if (!result && !selectFound)
        {
            scrollTop[workingWin]++;
            return false;
        }
        if (!result && !select_down[workingWin])
            selectIsLast = true;
        if (!select_down[workingWin])
        {
            const JSonContainer *pselect = dynamic_cast<const JSonContainer*>(selection[workingWin]);
            if (pselect && !pselect->empty())
                select_down[workingWin] = *(pselect->cbegin());
            else
            {
                const JSonElement *next = selection[workingWin]->findNext();
                select_down[workingWin] = next ? next : selection[workingWin];
            }
        }
        if (!select_up[workingWin])
            select_up[workingWin] = selection[workingWin];
        wrefresh(currentWin);
    }
    refresh();
    return true;
}

bool CurseSplitOutput::writeContainer(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *item)
{
    char childDelimiter[2];
    const int scrollTop = this->scrollTop[workingWin];

    if (dynamic_cast<const JSonObject *>(item))
        memcpy(childDelimiter, "{}", sizeof(*childDelimiter) * 2);
    else
        memcpy(childDelimiter, "[]", sizeof(*childDelimiter) * 2);

    if (collapsed.find((const JSonContainer *)item) != collapsed.end())
    {
        std::string ss;
        ss.append(&childDelimiter[0], 1).append(" ... ").append(&childDelimiter[1], 1);
        cursor.second += write(cursor.first, cursor.second, ss, 7, maxSize.first, CurseSplitOutput::getFlag(item));
    }
    else
    {
        cursor.second += write(cursor.first, cursor.second, childDelimiter[0], maxSize.first, CurseSplitOutput::getFlag(item));
        if (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1)
                return false;
        if (!writeContent(cursor, maxSize, (std::list<JSonElement *> *)item))
            return false;
        cursor.second += write(cursor.first, cursor.second, childDelimiter[1], maxSize.first, CurseSplitOutput::getFlag(item));
    }
    return (cursor.second - scrollTop < 0 || (unsigned)(cursor.second - scrollTop) <= maxSize.second -1);
}

bool CurseSplitOutput::writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, std::list<JSonElement*> *_item)
{
    JSonContainer *item = (JSonContainer *)_item;
    bool containerIsObject = (dynamic_cast<JSonObject *>(item) != nullptr);
    bool result = true;
    cursor.first += INDENT_LEVEL;
    const int scrollTop = this->scrollTop[workingWin];

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
                    if (!CurseOutput::writeKey(key, ent->lazystrlen(), "{ ... }", cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!CurseOutput::writeKey(key, ent->lazystrlen(), "[ ... ]", cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (!isContainer)
            {
                JSonElement *eContent = **ent;
                if (!writeKey(key, ent->lazystrlen(), eContent->stringify(), eContent->lazystrlen(), cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (((JSonContainer*)(**ent))->size() == 0)
            {
                if (dynamic_cast<const JSonObject *>(**ent) )
                {
                    if (!CurseOutput::writeKey(key, ent->lazystrlen(), "{ }", cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!CurseOutput::writeKey(key, ent->lazystrlen(), "[ ]", cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (cursor.second - scrollTop > 0 && (unsigned)(cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else
            {
                if (!writeKey(key, ent->lazystrlen(), cursor, maxSize, getFlag(ent)))
                    break;
                const JSonElement *saveSelection = selection[workingWin];
                if (selection[workingWin] == ent)
                    selection[workingWin] = **ent;
                cursor.first += INDENT_LEVEL /2;
                if (!redraw(cursor, maxSize, **ent))
                {
                    selection[workingWin] = saveSelection;
                    cursor.first -= INDENT_LEVEL /2;
                    return false;
                }
                selection[workingWin] = saveSelection;
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

bool CurseSplitOutput::writeKey(const std::string &key, const size_t keylen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags, unsigned int extraLen)
{
    if (cursor.second - scrollTop[workingWin] < 0)
    {
        cursor.second++;
        return true;
    }
    char oldType = flags.type();
    flags.type(OutputFlag::TYPE_OBJKEY);
    cursor.second += write(cursor.first, cursor.second, key, keylen, maxSize.first -extraLen -2, flags);
    flags.type(OutputFlag::TYPE_OBJ);
    CurseOutput::write(": ", flags);
    flags.type(oldType);
    return (cursor.second - scrollTop[workingWin] < 0 || (unsigned)(cursor.second - scrollTop[workingWin]) <= maxSize.second);
}

bool CurseSplitOutput::writeKey(const std::string &key, const size_t keylen, const std::string &after, size_t afterlen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags)
{
    if (cursor.second - scrollTop[workingWin] < 0)
    {
        cursor.second++;
        return true;
    }
    char oldType = flags.type();
    flags.type(OutputFlag::TYPE_OBJKEY);
    write(cursor.first, cursor.second, key, 0, 1, flags);
    flags.type(OutputFlag::TYPE_OBJ);
    CurseOutput::write(": ", flags);
    flags.type(oldType);
    CurseOutput::write(after.c_str(), flags);
    cursor.second += getNbLines(cursor.first +keylen +2 +afterlen, maxSize.first);
    return (cursor.second - scrollTop[workingWin] < 0 || (unsigned)(cursor.second - scrollTop[workingWin]) <= maxSize.second);
}

bool CurseSplitOutput::redraw(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, JSonElement *item)
{
    checkSelection(item, cursor);
    if (dynamic_cast<const JSonContainer*>(item))
    {
        if (!writeContainer(cursor, maxSize, (const JSonContainer *) item))
            return false;
    }
    else
    {
        cursor.second += CurseOutput::write(cursor.first, cursor.second, item, maxSize.first, CurseSplitOutput::getFlag(item));
        if (cursor.second - scrollTop[workingWin] > 0 && (unsigned)(cursor.second - scrollTop[workingWin]) > maxSize.second -1)
            return false;
    }
    return true;
}

unsigned int CurseSplitOutput::write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags)
{
    int offsetY = y - scrollTop[workingWin];
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

unsigned int CurseSplitOutput::write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags)
{
    int offsetY = y - scrollTop[workingWin];
    if (offsetY < 0)
        return 1;
    move(offsetY, x);
    CurseOutput::write(str, flags);
    return getNbLines(strlen +x, maxWidth);
}

void CurseSplitOutput::destroyAllSubWin()
{
    for (WINDOW *i: subwindows)
    {
        wborder(i, ' ', ' ', ' ',' ',' ',' ',' ',' ');
        wrefresh(i);
        delwin(i);
    }
    subwindows.clear();
}

const std::pair<unsigned int, unsigned int> CurseSplitOutput::getScreenSize() const
{
    std::pair<unsigned int, unsigned int> result = CurseOutput::getScreenSize();
    result.first /= nbInputs;
    return result;
}

void CurseSplitOutput::init()
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
    scrollTop[selectedWin] = 0;
}

void CurseSplitOutput::shutdown()
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

const OutputFlag CurseSplitOutput::getFlag(const JSonElement *e) const
{
    return getFlag(e, selection[workingWin]);
}

const OutputFlag CurseSplitOutput::getFlag(const JSonElement *item, const JSonElement *selection) const
{
    OutputFlag res;
    const JSonElement *i = dynamic_cast<const JSonObjectEntry*>(item) ? **((const JSonObjectEntry*)item) : item;

    res.selected(item == selection);
    res.searched(std::find(search_result[selectedWin].cbegin(), search_result[selectedWin].cend(), item) != search_result[selectedWin].cend());
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

