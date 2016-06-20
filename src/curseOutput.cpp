#include<iostream>

#include <unistd.h>
#include <ncurses.h>
#include <utility>
#include "curseOutput.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"
#include "optional.hpp"

CurseOutput::CurseOutput(JSonElement *root): data(root), selection(root), indentLevel(4)
{ }

CurseOutput::~CurseOutput()
{ }

void CurseOutput::run()
{
    init();
    do
    {
        redraw();
        refresh();
    } while(readInput());
    shutdown();
}

void CurseOutput::redraw()
{
    std::pair<int, int> screenSize;
    std::pair<int, int> cursor(topleft.second->getLevel() * indentLevel, 0);

    redraw(cursor, screenSize, topleft.second);
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
        selected = false;
        if (dynamic_cast<const JSonObject*>(item) != nullptr)
        {
            cursor.first += indentLevel /2;
            if (selection == item)
                selected = true;
            write(cursor.first, cursor.second, "{");
            selected = false;
            cursor.first += indentLevel /2;
            cursor.second++;
            for (JSonObject::const_iterator i = ((JSonObject *)item)->cbegin(); i != ((JSonObject *)item)->cend(); ++i)
            {
                const std::pair<std::string, JSonElement *> ipair = *i;
                if (selection == ipair.second)
                    selected = true;
                writeKey(ipair.first, cursor);
                redraw(cursor, maxSize, ipair.second);
                selected = false;
                cursor.first -= indentLevel;
            }
            cursor.first -= indentLevel /2;
            if (selection == item)
                selected = true;
            write(cursor.first, cursor.second, "}");
            selected = false;
            cursor.first -= indentLevel /2;
            cursor.second++;
        }
        else if (dynamic_cast<const JSonArray*>(item) != nullptr)
        {
            cursor.first += indentLevel /2;
            if (selection == item)
                selected = true;
            write(cursor.first, cursor.second, "[");
            selected = false;
            cursor.first += indentLevel /2;
            cursor.second++;
            for (JSonArray::const_iterator i = ((JSonArray *)item)->cbegin(); i != ((JSonArray *)item)->cend(); ++i)
                redraw(cursor, maxSize, *i);
            cursor.first -= indentLevel /2;
            if (selection == item)
                selected = true;
            write(cursor.first, cursor.second, "]");
            selected = false;
            cursor.first -= indentLevel /2;
            cursor.second++;
        }
        else
        {
            if (item == selection)
                selected = true;
            write(cursor.first, cursor.second, item->stringify());
            selected = false;
            cursor.second++;
        }
        t_nextKey next = findNext(item);
        if (next.isAbsent())
            break;
        if (next.value().first.isPresent())
            writeKey(next.value().first.value(), cursor);
        item = next.value().second;
    } while (true);
}

void CurseOutput::writeKey(const std::string &key, std::pair<int, int> &cursor)
{
    write(cursor.first, cursor.second, key +": ");
    cursor.first += indentLevel;
    cursor.second++;
}

void CurseOutput::write(const int &x, const int &y, JSonElement *item)
{
    std::string str = item->stringify();
    write(x, y, str);
}

void CurseOutput::write(const int &x, const int &y, const std::string &str)
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
    while (1)
    {
        char c = getch();
        std::string debug = std::to_string((int) c);
        mvprintw(0, 0, debug.c_str());
        mvprintw(1, 1, std::to_string(time(NULL)).c_str());
        refresh();
    }
    sleep(9);
    return false;
}

void CurseOutput::init()
{
    initscr();
    noecho();
    curs_set(false);
    cbreak();
    topleft.first = std::pair<unsigned int, unsigned int>(0, 0);
    topleft.second = data;
}

void CurseOutput::shutdown()
{
    endwin();
}

