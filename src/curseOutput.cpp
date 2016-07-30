#include <iostream>

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "curseOutput.hh"
#include "jsonPrimitive.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"

static CurseOutput *runningInst = nullptr;
class SelectionOutOfRange {};

CurseOutput::CurseOutput(JSonElement *root, const Params &p): data(root), selection(root), params(p)
{ }

CurseOutput::~CurseOutput()
{ }

void CurseOutput::run()
{
    runningInst = this;
    init();
    loop();
    shutdown();
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
        redraw();
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

static void _resizeFnc(int signo)
{
    if (!runningInst)
        return;
    runningInst->onsig(signo);
}

bool CurseOutput::redraw()
{
    std::pair<unsigned int, unsigned int> screenSize;
    std::pair<int, int> cursor;
    bool result;

    select_up = select_down = nullptr;
    selectFound = selectIsLast = selectIsFirst = false;
    getScreenSize(screenSize, cursor);
    cursor.first = cursor.second = 0;
    clear();
    try {
        result = redraw(cursor, screenSize, data, dynamic_cast<const JSonContainer *> (data));
    }
    catch (SelectionOutOfRange &e)
    {
        return false;
    }
    if (!result && !selectFound)
    {
        topleft++;
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

bool CurseOutput::redraw(const std::string &errorMsg)
{
    bool result = redraw();
    writeBottomLine(errorMsg, OutputFlag::SPECIAL_ERROR);
    return result;
}

bool CurseOutput::redraw(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const JSonElement *item, const JSonContainer *parent)
{
    checkSelection(item, parent, cursor);
    if (dynamic_cast<const JSonContainer*>(item))
    {
        if (!writeContainer(cursor, maxSize, (const JSonContainer *) item))
            return false;
    }
    else
    {
        cursor.second += write(cursor.first, cursor.second, item, maxSize.first, getFlag(item));
        if (cursor.second - topleft > 0 && (unsigned)(cursor.second - topleft) > maxSize.second -1)
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
        cursor.second += write(cursor.first, cursor.second, ss, maxSize.first, selection == item);
    }
    else
    {
        cursor.second += write(cursor.first, cursor.second, childDelimiter[0], maxSize.first, getFlag(item));
        if (cursor.second - topleft > 0 && (unsigned)(cursor.second - topleft) > maxSize.second -1)
                return false;
        if (!writeContent(cursor, maxSize, (const std::list<JSonElement *> *)item))
            return false;
        cursor.second += write(cursor.first, cursor.second, childDelimiter[1], maxSize.first, getFlag(item));
    }
    return (cursor.second - topleft < 0 || (unsigned)(cursor.second - topleft) <= maxSize.second -1);
}

bool CurseOutput::writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const std::list<JSonElement*> *_item)
{
    const JSonContainer *item = (const JSonContainer *)_item;
    bool containerIsObject = (dynamic_cast<const JSonObject *>(item) != nullptr);
    bool result = true;
    cursor.first += INDENT_LEVEL;

    for (JSonElement *i : *item)
    {
        result = false;
        if (containerIsObject)
        {
            JSonObjectEntry *ent = (JSonObjectEntry*) i;
            bool isContainer = (dynamic_cast<const JSonContainer *>(**ent) != nullptr);
            std::string key = ent->stringify();
            checkSelection(ent, (JSonContainer*) item, cursor);
            if (isContainer && collapsed.find((const JSonContainer*)(**ent)) != collapsed.cend())
            {
                if (dynamic_cast<const JSonObject *>(**ent))
                {
                    if (!writeKey(key, "{ ... }", cursor, maxSize, getFlag(ent)))
                        break;
                }
                else if (!writeKey(key, "[ ... ]", cursor, maxSize, getFlag(ent)) || (cursor.second - topleft > 0 && (unsigned)(cursor.second - topleft) > maxSize.second -1))
                    break;
            }
            else if (!isContainer)
            {
                if (!writeKey(key, ((**ent)->stringify()), cursor, maxSize, getFlag(ent)) || (cursor.second - topleft > 0 && (unsigned)(cursor.second - topleft) > maxSize.second -1))
                    break;
            }
            else if (((JSonContainer*)(**ent))->size() == 0)
            {
                if (dynamic_cast<const JSonObject *>(**ent) )
                {
                    if (!writeKey(key, "{ }", cursor, maxSize, getFlag(ent)))
                        break;
                }
                else if (!writeKey(key, "[ ]", cursor, maxSize, getFlag(ent)) || (cursor.second - topleft > 0 && (unsigned)(cursor.second - topleft) > maxSize.second -1))
                    break;
            }
            else
            {
                if (!writeKey(key, cursor, maxSize, selection == ent))
                    break;
                const JSonElement *saveSelection = selection;
                if (selection == ent)
                    selection = **ent;
                cursor.first += INDENT_LEVEL /2;
                if (!redraw(cursor, maxSize, **ent, (const JSonContainer *)item))
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
            if (!redraw(cursor, maxSize, i, (const JSonContainer *)item))
                break;
        }
        result = true;
    }
    cursor.first -= INDENT_LEVEL;
    //result will be false if for loop break'd at some time, true otherwise
    return result;
}

bool CurseOutput::writeKey(const std::string &key, const std::string &after, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags)
{
    if (cursor.second - topleft < 0)
    {
        cursor.second++;
        return true;
    }
    if (!writeKey(key, cursor, maxSize, flags, after.size()))
        return false;
    //TODO check result if write goes to new line
    write(after.c_str(), maxSize.first, flags);
    return true;
}

bool CurseOutput::writeKey(const std::string &key, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags, unsigned int extraLen)
{
    if (cursor.second - topleft < 0)
    {
        cursor.second++;
        return true;
    }
    char oldType = flags.type();
    flags.type(OutputFlag::TYPE_OBJKEY);
    write(cursor.first, cursor.second, key, maxSize.first -extraLen -2, flags);
    cursor.second ++;
    flags.type(OutputFlag::TYPE_OBJ);
    write(": ", maxSize.first, flags);
    flags.type(oldType);
    return (cursor.second - topleft < 0 || (unsigned)(cursor.second - topleft) <= maxSize.second);
}

unsigned int CurseOutput::write(const int &x, const int &y, const JSonElement *item, unsigned int maxWidth, OutputFlag flags)
{
    return write(x, y, item->stringify(), maxWidth, flags);
}

unsigned int CurseOutput::write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags)
{
    int offsetY = y - topleft;
    if (offsetY < 0)
        return 1;
    if (flags.selected())
        attron(A_REVERSE | A_BOLD);
    char color = OutputFlag::SPECIAL_NONE;

    if (params.colorEnabled() && search_pattern.size() == 1 && search_pattern.c_str()[0] == item)
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

void CurseOutput::write(const std::string &str, unsigned int maxWidth, const OutputFlag flags) const
{
    char color = OutputFlag::SPECIAL_NONE;
    if (params.colorEnabled() && !search_pattern.empty() && str.find(search_pattern) != str.npos)
        color = OutputFlag::SPECIAL_SEARCH;
    else if (colors.find(flags.type()) != colors.end())
        color = flags.type();

    if (color != OutputFlag::SPECIAL_NONE)
        attron(COLOR_PAIR(color));
    if (flags.selected())
        attron(A_REVERSE | A_BOLD);

    printw("%s", str.c_str());
    attroff(A_REVERSE | A_BOLD);
    if (color != OutputFlag::SPECIAL_NONE)
        attroff(COLOR_PAIR(color));
}

unsigned int CurseOutput::write(const int &x, const int &y, const std::string &str, unsigned int maxWidth, const OutputFlag flags)
{
    int offsetY = y - topleft;
    if (offsetY < 0)
        return 1;
    move(offsetY, x);
    write(str, maxWidth, flags);
    return getNbLines(str.size() +x, maxWidth);
}

unsigned int CurseOutput::getNbLines(float nbChar, unsigned int maxWidth)
{
    float nLine = nbChar / maxWidth;
    if (nLine == (unsigned int) nLine)
        return nLine;
    return nLine +1;
}

void CurseOutput::getScreenSize(std::pair<unsigned int, unsigned int> &screenSize) const
{
    getmaxyx(stdscr, screenSize.second, screenSize.first);
}

void CurseOutput::getScreenSize(std::pair<unsigned int, unsigned int> &screenSize, std::pair<int, int> &bs) const
{
    getScreenSize(screenSize);
    getbegyx(stdscr, bs.second, bs.first);
}

const OutputFlag CurseOutput::getFlag(const JSonElement *item) const
{
    OutputFlag res;
    const JSonElement *i = dynamic_cast<const JSonObjectEntry*>(item) ? **((const JSonObjectEntry*)item) : item;

    res.selected(item == selection);
    if (dynamic_cast<const JSonPrimitive<std::string> *>(i))
        res.type(OutputFlag::TYPE_STRING);
    else if (dynamic_cast<const JSonPrimitive<bool> *>(i))
        res.type(OutputFlag::TYPE_BOOL);
    else if (dynamic_cast<const AJSonPrimitive *>(i))
        res.type(OutputFlag::TYPE_NUMBER);
    else if (dynamic_cast<const JSonObject*>(i))
        res.type(OutputFlag::TYPE_OBJ);
    else if (dynamic_cast<const JSonArray*>(i))
        res.type(OutputFlag::TYPE_ARR);
    return res;
}

void CurseOutput::checkSelection(const JSonElement *item, const JSonElement *parent, const std::pair<int, int> &cursor)
{
    if (!selectFound)
    {
        if (selection == item)
        {
            if (cursor.second == topleft)
                selectIsFirst = true;
            else if (cursor.second < topleft)
            {
                topleft = cursor.second;
                throw SelectionOutOfRange(); //break and restart painting
            }
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
                if (selectIsFirst && topleft)
                    topleft = std::max(topleft -3, 0);
                else
                    selection = select_up;
                return true;

            case KEY_DOWN:
            case 'j':
            case 'J':
                if (selectIsLast)
                    topleft += 2;
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
                search_pattern = search();

            case 'n':
            case 'N':
                jumpToNextSearch(true, true, selection);
                // jumpToNextSearch have its own redraw, so no need to return true here
                break;
        }
    }
    return false;
}

//TODO move to JSonElement ?
bool CurseOutput::jumpToNextSearch(bool scanParent, bool redraw, const JSonElement *initial_selection)
{
    bool found = false;
    const JSonElement *current = selection;
    const JSonElement *prev = nullptr;

    while (current && !found)
    {
        if (dynamic_cast<const JSonContainer *> (current) == nullptr)
        {
            //ObjectEntry or Primitive
            const JSonElement* entryChild = nullptr;
            char isObjectEntry = 0;
            if (dynamic_cast<const JSonObjectEntry *>(current) != nullptr)
            {
                entryChild = **((const JSonObjectEntry*)current);
                if (dynamic_cast<const JSonContainer *> (entryChild) == nullptr)
                    isObjectEntry = 1;
                else
                    isObjectEntry = 2;
            }
            const std::string str = current->stringify();
            std::string strEntry;
            if (isObjectEntry == 1)
                strEntry = entryChild->stringify();
            if (current != initial_selection && entryChild != initial_selection && current->match(search_pattern))
            {
                selection = current;
                found = true;
                break;
            }
            if (isObjectEntry == 2)
            {
                const JSonElement *_selection = selection;
                selection = entryChild;
                if (jumpToNextSearch(false, false, initial_selection))
                {
                    found = true;
                    break;
                }
                selection = _selection;
            }
        }
        else
        {
            //Object or array
            for (const JSonElement *i : *(const JSonContainer*)current)
            {
                if (prev != nullptr)
                {
                    if (prev != i)
                        continue;
                    else
                    {
                        prev = nullptr;
                        continue;
                    }
                }
                const JSonElement *_selection = selection;
                selection = i;
                if (jumpToNextSearch(false, false, initial_selection))
                {
                    found = true;
                    break;
                }
                selection = _selection;
            }
        }
        if (!scanParent)
            break;
        prev = current;
        current = current->getParent();
    }
    bool foundAfterLoop = false;
    if (!found && scanParent && selection != data)
    {
        const JSonElement *_selection = selection;
        selection = data;
        if (jumpToNextSearch(false, false, initial_selection))
            foundAfterLoop = true;
        else
            selection = _selection;
    }
    if (!redraw)
        return found;
    if (foundAfterLoop || (!found && selection->match(search_pattern)))
    {
        this->redraw("Search hit BOTTOM, continuing at TOP");
        return true;
    }
    if (!found)
    {
        this->redraw("Pattern not found");
        return false;
    }
    unfold(selection);
    this->redraw();
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

const std::string CurseOutput::search()
{
    std::string buffer;

    curs_set(true);
    keypad(stdscr, false);
    wtimeout(stdscr, -1);
    while (true)
    {
        int c;

        clear();
        redraw();
        writeBottomLine('/' +buffer, OutputFlag::SPECIAL_SEARCH);
        refresh();
        c = getch();
        if (c == '\n')
            break;
        else if (c == '\b' || c == 127)
            buffer.pop_back();
        else
            buffer += c;
    }
    wtimeout(stdscr, 150);
    keypad(stdscr, true);
    curs_set(false);

    return buffer;
}

void CurseOutput::writeBottomLine(const std::string &buffer, short color) const
{
    std::pair<unsigned int, unsigned int> screenSize;
    getScreenSize(screenSize);
    size_t bufsize = buffer.size();
    if (params.colorEnabled())
        attron(COLOR_PAIR(color));
    mvprintw(screenSize.second -1, 0, "%s%*c", buffer.c_str(), screenSize.first - bufsize, ' ');
    move(screenSize.second -1, bufsize);
    if (params.colorEnabled())
        attroff(COLOR_PAIR(color));
}

void CurseOutput::init()
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
        init_pair(OutputFlag::TYPE_STRING, COLOR_CYAN, COLOR_BLACK);
        init_pair(OutputFlag::TYPE_OBJKEY, COLOR_CYAN, COLOR_BLACK);
        init_pair(OutputFlag::SPECIAL_SEARCH, COLOR_WHITE, COLOR_BLUE);
        init_pair(OutputFlag::SPECIAL_ERROR, COLOR_WHITE, COLOR_RED);
        colors.insert(OutputFlag::TYPE_NUMBER);
        colors.insert(OutputFlag::TYPE_BOOL);
        colors.insert(OutputFlag::TYPE_STRING);
        colors.insert(OutputFlag::TYPE_OBJKEY);
    }

    signal(SIGWINCH, _resizeFnc);
    signal(SIGINT, _resizeFnc);
    signal(SIGTERM, _resizeFnc);
    signal(SIGKILL, _resizeFnc);
    topleft = 0;
}

void CurseOutput::shutdown()
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

