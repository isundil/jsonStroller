#include<iostream>

#include <unistd.h>
#include <ncurses.h>
#include <utility>
#include "curseOutput.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"

CurseOutput::CurseOutput(JSonElement *root): data(root), indentLevel(4)
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
        sleep(9);
    } while(readInput());
    shutdown();
}

void CurseOutput::redraw()
{
    std::pair<int, int> screenSize;
    std::pair<int, int> cursor(0, 0);

    redraw(cursor, screenSize, data);
}

void CurseOutput::redraw(std::pair<int, int> &cursor, const std::pair<int, int> &maxSize, const JSonElement *item)
{
    if (dynamic_cast<const JSonObject*>(item) != nullptr)
    {
        for (JSonObject::const_iterator i = ((JSonObject *)item)->cbegin(); i != ((JSonObject *)item)->cend(); ++i)
        {
            const std::pair<std::string, JSonElement *> ipair = *i;
            std::string key = ipair.first;
            write(cursor.first, cursor.second, key +": ");
            cursor.first += indentLevel;
            cursor.second++;
            redraw(cursor, maxSize, ipair.second);
            cursor.first -= indentLevel;
        }
    }
    else if (dynamic_cast<const JSonArray*>(item) != nullptr)
    {
    }
    else
    {
        write(cursor.first, cursor.second, item->stringify());
        cursor.second++;
    }
}

void CurseOutput::write(const int &x, const int &y, JSonElement *item)
{
    std::string str = item->stringify();
    write(x, y, str);
}

void CurseOutput::write(const int &x, const int &y, const std::string &str)
{
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
    //TODO
    return false;
}

void CurseOutput::init()
{
    initscr();
    noecho();
    curs_set(false);
}

void CurseOutput::shutdown()
{
    endwin();
}

