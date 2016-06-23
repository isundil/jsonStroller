#include<iostream>

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
        refresh();
    } while(readInput());
}

bool CurseOutput::onsig(int signo)
{
    switch (signo)
    {
    case SIGWINCH:
        clear();
        redraw();
        refresh();
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
    std::pair<int, int> cursor(topleft.second->getLevel() * indentLevel, 0);

    select_up = select_down = nullptr;
    selectFound = false;
    getScreenSize(screenSize);
    redraw(cursor, screenSize, topleft.second);
    move(screenSize.second, screenSize.first);
    if (!select_down)
        select_down = selection;
    if (!select_up)
        select_up = selection;
}

void CurseOutput::getScreenSize(std::pair<int, int> &ss)
{
    getmaxyx(stdscr, ss.second, ss.first);
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

void CurseOutput::redraw(std::pair<int, int> &cursor, const std::pair<int, int> &maxSize, const JSonElement *item)
{
    do
    {
        if (dynamic_cast<const JSonObject*>(item) != nullptr)
        {
            cursor.first += indentLevel /2;
            if (selection == item)
                selectFound = true;
            else if (!selectFound)
                select_up = item;
            else if (!select_down)
                select_down = item;
            write(cursor.first, cursor.second, "{", selection == item);
            cursor.second++;
            for (JSonObject::const_iterator i = ((JSonObject *)item)->cbegin(); i != ((JSonObject *)item)->cend(); ++i)
            {
                const std::pair<std::string, JSonElement *> ipair = *i;
                cursor.first += indentLevel /2;
                writeKey(ipair.first, cursor, selection == ipair.second);
                cursor.first -= indentLevel /2;
                redraw(cursor, maxSize, ipair.second);
                cursor.first -= indentLevel;
            }
            write(cursor.first, cursor.second, "}", selection == item);
            cursor.first -= indentLevel /2;
            cursor.second++;
        }
        else if (dynamic_cast<const JSonArray*>(item) != nullptr)
        {
            cursor.first += indentLevel /2;
            if (selection == item)
                selectFound = true;
            else if (!selectFound)
                select_up = item;
            else if (!select_down)
                select_down = item;
            write(cursor.first, cursor.second, "[", selection == item);
            cursor.first += indentLevel /2;
            cursor.second++;
            for (JSonArray::const_iterator i = ((JSonArray *)item)->cbegin(); i != ((JSonArray *)item)->cend(); ++i)
                redraw(cursor, maxSize, *i);
            cursor.first -= indentLevel /2;
            write(cursor.first, cursor.second, "]", selection == item);
            cursor.first -= indentLevel /2;
            cursor.second++;
        }
        else
        {
            if (item == selection)
                selectFound = true;
            else if (!selectFound)
                select_up = item;
            else if (!select_down)
                select_down = item;
            write(cursor.first, cursor.second, item, selection == item);
            cursor.second++;
        }
        t_nextKey next = findNext(item);
        if (next.isAbsent())
            break;
        item = next.value().second;
        if (next.value().first.isPresent())
            writeKey(next.value().first.value(), cursor, selection == item);
    } while (true);
}

void CurseOutput::writeKey(const std::string &key, std::pair<int, int> &cursor, bool selected)
{
    write(cursor.first, cursor.second, key +": ", selected);
    cursor.first += indentLevel;
    cursor.second++;
}

void CurseOutput::write(const int &x, const int &y, const JSonElement *item, bool selected)
{
    write(x, y, item->stringify(), selected);
}

void CurseOutput::write(const int &x, const int &y, const std::string &str, bool selected)
{
    if (selected)
    {
        attron(A_REVERSE | A_BOLD);
        mvprintw(y, x, str.c_str());
        attroff(A_REVERSE | A_BOLD);
    }
    else
        mvprintw(y, x, str.c_str());
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
        c = wgetch(stdscr);

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
        }
    }
    return false;
}

void CurseOutput::init()
{
    initscr();
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
    if (screen)
    {
        delscreen(screen);
        endwin();
        if (screen_fd)
        {
            fclose(screen_fd);
            screen_fd = nullptr;
        }
        screen = nullptr;
    }
}

