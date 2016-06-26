#include<iostream>

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <utility>

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

void CurseOutput::redraw()
{
    std::pair<int, int> screenSize;
    std::pair<int, int> cursor;

    select_up = select_down = select_parent = nullptr;
    selectFound = false;
    getScreenSize(screenSize, cursor);
    cursor.first += topleft.second->getLevel() * indentLevel;
    clear();
    redraw(cursor, screenSize, topleft.second, dynamic_cast<const JSonContainer *> (topleft.second));
    move(screenSize.second, screenSize.first);
    if (!select_down)
        select_down = selection;
    if (!select_up)
        select_up = selection;
    if (!select_parent)
        select_parent = selection;
    refresh();
}

void CurseOutput::getScreenSize(std::pair<int, int> &ss, std::pair<int, int> &bs)
{
    getmaxyx(stdscr, ss.second, ss.first);
    getbegyx(stdscr, bs.second, bs.first);
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
                    return t_nextKey::empty(); // Last item
                return t_nextKey(std::pair<Optional<const std::string>, const JSonElement *>((*it).first, (*it).second));
            }
            it++;
        }
        return t_nextKey::empty();
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
                    return t_nextKey::empty(); // Last item
                return t_nextKey(std::pair<Optional<const std::string>, const JSonElement *>(Optional<const std::string>::empty(), *it));
            }
            it++;
        }
        return t_nextKey::empty();
    }
    return t_nextKey::empty(); // Primitive, can't have child (impossible)
}

bool CurseOutput::redraw(std::pair<int, int> &cursor, const std::pair<int, int> &maxSize, const JSonElement *item, const JSonContainer *parent)
{
    do
    {
        checkSelection(item, parent);
        if (dynamic_cast<const JSonContainer*>(item))
        {
            if (!writeContainer(cursor, maxSize, (const JSonContainer *) item))
                return false;
        }
        else
        {
            write(cursor.first, cursor.second, item, selection == item);
            if (++cursor.second > maxSize.second)
                return false;
        }
        t_nextKey next = findNext(item);
        if (next.isAbsent())
            break;
        item = next.value().second;
        if (next.value().first.isPresent())
            if (!writeKey(next.value().first.value(), cursor, maxSize.second, selection == item))
                return false;
    } while (true);
    return true;
}

void CurseOutput::checkSelection(const JSonElement *item, const JSonElement *parent)
{
    if (selection == item)
    {
        select_parent = parent;
        selectFound = true;
    }
    else if (!selectFound)
        select_up = item;
    else if (!select_down)
        select_down = item;
}

bool CurseOutput::writeContainer(std::pair<int, int> &cursor, const std::pair<int, int> &maxSize, const JSonContainer *item)
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
        write(cursor.first, cursor.second, ss, selection == item);
        cursor.first -= indentLevel /2;
        return !(++cursor.second > maxSize.second);
    }

    write(cursor.first, cursor.second, childDelimiter[0], selection == item);
    if (++cursor.second > maxSize.second)
        return false;

    if (isObject)
        writeContent(cursor, maxSize, (const JSonObject *)item);
    else
        writeContent(cursor, maxSize, (const JSonArray *)item);
    write(cursor.first, cursor.second, childDelimiter[1], selection == item);
    cursor.first -= indentLevel /2;
    return !(++cursor.second > maxSize.second);
}

bool CurseOutput::writeContent(std::pair<int, int> &cursor, const std::pair<int, int> &maxSize, const JSonArray *item)
{
    cursor.first += indentLevel /2;
    for (JSonArray::const_iterator i = ((JSonArray *)item)->cbegin(); i != ((JSonArray *)item)->cend(); ++i)
        if (!redraw(cursor, maxSize, *i, (JSonContainer *)item))
            return false;
    cursor.first -= indentLevel /2;
    return true;
}

bool CurseOutput::writeContent(std::pair<int, int> &cursor, const std::pair<int, int> &maxSize, const JSonObject *item)
{
    for (JSonObject::const_iterator i = ((JSonObject *)item)->cbegin(); i != ((JSonObject *)item)->cend(); ++i)
    {
        const std::pair<std::string, JSonElement *> ipair = *i;
        cursor.first += indentLevel /2;
        if (dynamic_cast<const JSonContainer *>(ipair.second) == nullptr
                || ((const JSonContainer *) ipair.second)->size() == 0)
        {
            checkSelection(ipair.second, item);
            write(cursor.first, cursor.second, ipair.first +": " +ipair.second->stringify(), selection == ipair.second);
            cursor.first -= indentLevel /2;
            if (++cursor.second > maxSize.second)
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
            checkSelection(ipair.second, item);
            write(cursor.first, cursor.second, ss, selection == ipair.second);
            cursor.first -= indentLevel /2;
            if (++cursor.second > maxSize.second)
                return false;
        }
        else
        {
            if (!writeKey(ipair.first, cursor, maxSize.second, selection == ipair.second))
                return false;
            cursor.first -= indentLevel /2;
            if (!redraw(cursor, maxSize, ipair.second, (JSonContainer *) item))
                return false;
            cursor.first -= indentLevel;
        }
    }
    return true;
}

bool CurseOutput::writeKey(const std::string &key, std::pair<int, int> &cursor, const int maxSize, bool selected)
{
    write(cursor.first, cursor.second, key +": ", selected);
    cursor.first += indentLevel;
    return !(++cursor.second > maxSize);
}

void CurseOutput::write(const int &x, const int &y, const JSonElement *item, bool selected)
{
    write(x, y, item->stringify(), selected);
}

void CurseOutput::write(const int &x, const int &y, const char item, bool selected)
{
    if (selected)
    {
        attron(A_REVERSE | A_BOLD);
        mvprintw(y, x, "%c", item);
        attroff(A_REVERSE | A_BOLD);
    }
    else
        mvprintw(y, x, "%c", item);
}

void CurseOutput::write(const int &x, const int &y, const char *str, bool selected)
{
    if (selected)
    {
        attron(A_REVERSE | A_BOLD);
        mvprintw(y, x, "%s", str);
        attroff(A_REVERSE | A_BOLD);
    }
    else
        mvprintw(y, x, "%s", str);
}

void CurseOutput::write(const int &x, const int &y, const std::string &str, bool selected)
{
    write(x, y, str.c_str(), selected);
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
                selection = select_down;
                return true;

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
                    selection = select_parent;
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
    topleft.first = std::pair<unsigned int, unsigned int>(0, 0);
    topleft.second = data;
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

