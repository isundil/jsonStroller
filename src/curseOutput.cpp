#include <iostream>

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "curseOutput.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"

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
        while (!redraw())
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

    for (std::list<JSonElement *>::const_iterator i = item->cbegin(); i != item->cend(); ++i)
    {
        result = false;
        if (containerIsObject)
        {
            JSonObjectEntry *ent = (JSonObjectEntry*) *i;
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
            if (!redraw(cursor, maxSize, *i, (const JSonContainer *)item))
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
    bool color = (colors.find(flags.type()) != colors.end());
    if (color)
        attron(COLOR_PAIR(flags.type()));
    mvprintw(offsetY, x, "%c", item);
    attroff(A_REVERSE | A_BOLD);
    if (color)
        attroff(COLOR_PAIR(flags.type()));
    return getNbLines(x +1, maxWidth);
}

void CurseOutput::write(const char *str, unsigned int maxWidth, const OutputFlag flags) const
{
    if (flags.selected())
        attron(A_REVERSE | A_BOLD);
    bool color = (colors.find(flags.type()) != colors.end());
    if (color)
        attron(COLOR_PAIR(flags.type()));
    printw("%s", str);
    attroff(A_REVERSE | A_BOLD);
    if (color)
        attroff(COLOR_PAIR(flags.type()));
}

unsigned int CurseOutput::write(const int &x, const int &y, const char *str, unsigned int maxWidth, const OutputFlag flags)
{
    int offsetY = y - topleft;
    if (offsetY < 0)
        return 1;
    if (flags.selected())
        attron(A_REVERSE | A_BOLD);
    bool color = (colors.find(flags.type()) != colors.end());
    if (color)
        attron(COLOR_PAIR(flags.type()));
    mvprintw(offsetY, x, "%s", str);
    attroff(A_REVERSE | A_BOLD);
    if (color)
        attroff(COLOR_PAIR(flags.type()));
    return getNbLines(strlen(str) +x, maxWidth);
}

unsigned int CurseOutput::write(const int &x, const int &y, const std::string &str, unsigned int maxWidth, const OutputFlag flags)
{
    return write(x, y, str.c_str(), maxWidth, flags);
}

unsigned int CurseOutput::getNbLines(float nbChar, unsigned int maxWidth)
{
    float nLine = nbChar / maxWidth;
    if (nLine == (unsigned int) nLine)
        return nLine;
    return nLine +1;
}

void CurseOutput::getScreenSize(std::pair<unsigned int, unsigned int> &screenSize, std::pair<int, int> &bs) const
{
    getmaxyx(stdscr, screenSize.second, screenSize.first);
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
    else if (dynamic_cast<const AJsonPrimitive *>(i))
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
        }
    }
    return false;
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

