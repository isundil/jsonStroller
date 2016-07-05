#include <algorithm>
#include <iostream>
#include <utility>

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "curseOutput.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"
#include "optional.hpp"

static CurseOutput *runningInst = nullptr;

CurseOutput::CurseOutput(JSonElement *root): data(root), selection(root), indentLevel(4)
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
        redraw();
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
    cursor.first = 0;
    cursor.second = 0;
    clear();
    result = redraw(cursor, screenSize, data, dynamic_cast<const JSonContainer *> (data));
    if (!result && !select_down)
        selectIsLast = true;
    if (!select_down)
        select_down = selection;
    if (!select_up)
        select_up = selection;
    refresh();
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
        cursor.second += write(cursor.first, cursor.second, item, maxSize.first, selection == item);
        if (cursor.second - topleft> maxSize.second -1)
            return false;
    }
    return true;
}

bool CurseOutput::writeContainer(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *item)
{
    char childDelimiter[2];
    bool isObject = dynamic_cast<const JSonObject *>(item);

    if (isObject)
        memcpy(childDelimiter, "{}", sizeof(*childDelimiter) * 2);
    else
        memcpy(childDelimiter, "[]", sizeof(*childDelimiter) * 2);

    cursor.first += indentLevel /2;
    if (collapsed.find((const JSonContainer *)item) != collapsed.end())
    {
        std::string ss;
        ss.append(&childDelimiter[0], 1).append(" ... ").append(&childDelimiter[1], 1);
        cursor.second += write(cursor.first, cursor.second, ss, maxSize.first, selection == item);
    }
    else
    {
        cursor.second += write(cursor.first, cursor.second, childDelimiter[0], maxSize.first, selection == item);
        if (cursor.second - topleft > maxSize.second -1)
                return false;
        if (isObject)
        {
            if (!writeContent(cursor, maxSize, (const JSonObject *)item))
                return false;
        }
        else
        {
            if (!writeContent(cursor, maxSize, (const JSonArray *)item))
                return false;
        }
        cursor.second += write(cursor.first, cursor.second, childDelimiter[1], maxSize.first, selection == item);
    }
    cursor.first -= indentLevel /2;
    return (cursor.second - topleft <= maxSize.second -1);
}

bool CurseOutput::writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const JSonArray *item)
{
    cursor.first += indentLevel /2;
    for (JSonArray::const_iterator i = ((JSonArray *)item)->cbegin(); i != ((JSonArray *)item)->cend(); ++i)
        if (!redraw(cursor, maxSize, *i, (JSonContainer *)item))
            return false;
    cursor.first -= indentLevel /2;
    return true;
}

bool CurseOutput::writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const JSonObject *item)
{
    for (JSonObject::const_iterator i = ((JSonObject *)item)->cbegin(); i != ((JSonObject *)item)->cend(); ++i)
    {
        const std::pair<std::string, JSonElement *> ipair = *i;
        cursor.first += indentLevel /2;
        if (dynamic_cast<const JSonContainer *>(ipair.second) == nullptr
                || ((const JSonContainer *) ipair.second)->size() == 0)
        {
            checkSelection(ipair.second, item, cursor);
            cursor.second += write(cursor.first, cursor.second, ipair.first +": " +ipair.second->stringify(), maxSize.first, selection == ipair.second);
            cursor.first -= indentLevel /2;
            if (cursor.second - topleft > maxSize.second -1)
                return false;
        }
        else if (collapsed.find((const JSonContainer *)ipair.second) != collapsed.end())
        {
            char childDelimiter[2];
            if (dynamic_cast<const JSonObject *>(ipair.second))
                memcpy(childDelimiter, "{}", sizeof(*childDelimiter) * 2);
            else
                memcpy(childDelimiter, "[]", sizeof(*childDelimiter) * 2);
            std::string ss = ipair.first;
            ss.append(": ").append(&childDelimiter[0], 1).append(" ... ").append(&childDelimiter[1], 1);
            checkSelection(ipair.second, item, cursor);
            cursor.second += write(cursor.first, cursor.second, ss, maxSize.first, selection == ipair.second);
            cursor.first -= indentLevel /2;
            if (cursor.second - topleft > maxSize.second -1)
                return false;
        }
        else
        {
            if (!writeKey(ipair.first, cursor, maxSize, selection == ipair.second))
                return false;
            cursor.first -= indentLevel /2;
            if (!redraw(cursor, maxSize, ipair.second, (JSonContainer *) item))
                return false;
            cursor.first -= indentLevel;
        }
    }
    return true;
}

bool CurseOutput::writeKey(const std::string &key, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, bool selected)
{
    cursor.second += write(cursor.first, cursor.second, key +": ", maxSize.first, selected);
    cursor.first += indentLevel;
    return (cursor.second - topleft <= maxSize.second -1);
}

unsigned int CurseOutput::write(const int &x, const int &y, const JSonElement *item, unsigned int maxWidth, bool selected)
{
    return write(x, y, item->stringify(), maxWidth, selected);
}

unsigned int CurseOutput::write(const int &x, const int &y, const char item, unsigned int maxWidth, bool selected)
{
    int offsetY = y - topleft;
    if (offsetY < 0)
        return 1;
    if (selected)
    {
        attron(A_REVERSE | A_BOLD);
        mvprintw(offsetY, x, "%c", item);
        attroff(A_REVERSE | A_BOLD);
    }
    else
        mvprintw(offsetY, x, "%c", item);
    return getNbLines(x +1, maxWidth);
}

unsigned int CurseOutput::getNbLines(float nbChar, unsigned int maxWidth)
{
    float nLine = nbChar / maxWidth;
    if (nLine == (unsigned int) nLine)
        return nLine;
    return nLine +1;
}

unsigned int CurseOutput::write(const int &x, const int &y, const char *str, unsigned int maxWidth, bool selected)
{
    int offsetY = y - topleft;
    if (offsetY < 0)
        return 1;
    if (selected)
    {
        attron(A_REVERSE | A_BOLD);
        mvprintw(offsetY, x, "%s", str);
        attroff(A_REVERSE | A_BOLD);
    }
    else
        mvprintw(offsetY, x, "%s", str);
    return getNbLines(strlen(str) +x, maxWidth);
}

unsigned int CurseOutput::write(const int &x, const int &y, const std::string &str, unsigned int maxWidth, bool selected)
{
    return write(x, y, str.c_str(), maxWidth, selected);
}

void CurseOutput::getScreenSize(std::pair<unsigned int, unsigned int> &screenSize, std::pair<int, int> &bs) const
{
    getmaxyx(stdscr, screenSize.second, screenSize.first);
    getbegyx(stdscr, bs.second, bs.first);
}

CurseOutput::t_nextKey CurseOutput::findPrev(const JSonElement *item)
{
    const JSonContainer *parent = item->getParent();
    if (parent == nullptr)
        return t_nextKey::empty(); // Root node, can't have brothers
    if (dynamic_cast<const JSonObject *>(parent) != nullptr)
    {
        const JSonObject *oParent = (const JSonObject *) parent;
        JSonObject::const_iterator it = oParent->cbegin();
        if ((*it).second == item)
            return t_nextKey::empty();
        std::string prevKey = (*it).first;
        const JSonElement *prevElem = (*it).second;
        while ((++it) != oParent->cend())
        {
            if ((*it).second == item)
                return t_nextKey(std::pair<Optional<const std::string>, const JSonElement *>(prevKey, prevElem));
            prevKey = (*it).first;
            prevElem = (*it).second;
        }
        return t_nextKey::empty();
    }
    if (dynamic_cast<const JSonArray *>(parent) != nullptr)
    {
        const JSonArray *aParent = (const JSonArray *) parent;
        JSonArray::const_iterator it = aParent->cbegin();
        const JSonElement *prevElem = (*it);
        if (prevElem == item)
            return t_nextKey::empty();
        while ((++it) != aParent->cend())
        {
            if (*it == item)
                return t_nextKey(std::pair<Optional<const std::string>, const JSonElement *>(Optional<const std::string>::empty(), prevElem));
            prevElem = (*it);
        }
        return t_nextKey::empty();
    }
    return t_nextKey::empty(); // Primitive, can't have child (impossible)
}

CurseOutput::t_nextKey CurseOutput::findNext(const JSonElement *item)
{
    const JSonContainer *parent = item->getParent();
    if (parent == nullptr)
        return t_nextKey::empty(); // Root node, can't have brothers
    if (dynamic_cast<const JSonObject *>(parent) != nullptr)
    {
        const JSonObject *oParent = (const JSonObject *) parent;
        JSonObject::const_iterator it = oParent->cbegin();
        while (it != oParent->cend())
        {
            if ((*it).second == item)
            {
                it++;
                if (it == oParent->cend())
                    return findNext(oParent); // Last item
                return t_nextKey(std::pair<Optional<const std::string>, const JSonElement *>((*it).first, (*it).second));
            }
            it++;
        }
        return findNext(oParent);
    }
    if (dynamic_cast<const JSonArray *>(parent) != nullptr)
    {
        const JSonArray *aParent = (const JSonArray *) parent;
        JSonArray::const_iterator it = aParent->cbegin();
        while (it != aParent->cend())
        {
            if (*it == item)
            {
                it++;
                if (it == aParent->cend())
                    return findNext(aParent); // Last item
                return t_nextKey(std::pair<Optional<const std::string>, const JSonElement *>(Optional<const std::string>::empty(), *it));
            }
            it++;
        }
        return findNext(aParent);
    }
    return t_nextKey::empty(); // Primitive, can't have child (impossible)
}

void CurseOutput::checkSelection(const JSonElement *item, const JSonElement *parent, const std::pair<int, int> &cursor)
{
    if (selection == item)
    {
        if (cursor.second <= topleft)
            selectIsFirst = true;
        selectFound = true;
    }
    else if (!selectFound)
        select_up = item;
    else if (!select_down)
        select_down = item;
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
                else
                    selection = select_down;
                return true;

            case KEY_PPAGE:
            {
                const t_nextKey brother = findPrev(selection);
                if (brother.isAbsent())
                {
                    const JSonContainer *parent = selection->getParent();
                    if (parent)
                        selection = parent;
                    else
                        break;
                }
                else
                    selection = brother.value().second;
                return true;
                break;
            }

            case KEY_NPAGE:
            {
                const t_nextKey brother = findNext(selection);
                if (brother.isPresent())
                {
                    selection = brother.value().second;
                    return true;
                }
                break;
            }

            case 'l':
            case 'L':
            case KEY_RIGHT:
            {
                const JSonContainer *_selection = dynamic_cast<const JSonContainer *>(selection);
                if (!_selection)
                    break;

                if (collapsed.erase((const JSonContainer *) selection))
                    return true;
                if (!_selection->size())
                    break;
                selection = _selection->firstChild();
                return true;
            }

            case 'h':
            case 'H':
            case KEY_LEFT:
            {
                const JSonContainer *_selection = dynamic_cast<const JSonContainer *>(selection);
                if (!_selection
                        || collapsed.find((const JSonContainer *) selection) != collapsed.end()
                        || (_selection && _selection->size() == 0))
                {
                    const JSonContainer *parent = selection->getParent();
                    selection = parent ? parent : selection;
                }
                else if (_selection)
                    collapsed.insert((const JSonContainer *)selection);
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
        screen = newterm(nullptr, stdout, stdin);
    wtimeout(stdscr, 150);
    cbreak();
    clear();
    noecho();
    curs_set(false);
    keypad(stdscr, true);

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

