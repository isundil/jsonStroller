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

CurseOutput::CurseOutput(const Params &p): params(p)
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
        inputResult r = evalKey(InputSequence::read());
        if (r == inputResult::redraw)
            return true;
        if (r == inputResult::quit)
            return false;
        // else nextInput;
    }
    return false;
}

inputResult CurseOutput::evalKey(const InputSequence &c)
{
    switch (c.key())
    {
        case 'q':
        case 'Q':
            return inputResult::quit;

        case KEY_UP:
        case 'K':
        case 'k':
            return selectUp();

        case KEY_DOWN:
        case 'j':
        case 'J':
            return selectDown();

        case KEY_PPAGE:
            return selectPUp();

        case KEY_NPAGE:
            return selectPDown();

        case 'l':
        case 'L':
        case KEY_RIGHT:
            return expandSelection();

        case 'h':
        case 'H':
        case KEY_LEFT:
            return collapseSelection();

        case '/':
            return initSearch();

        case 'n':
        case 'N':
            return nextResult();
    }
    return inputResult::nextInput;
}

bool CurseOutput::redraw(const std::string &errorMsg)
{
    bool result = redraw();
    writeBottomLine(errorMsg, OutputFlag::SPECIAL_ERROR);
    return result;
}

bool CurseOutput::writeKey(const std::string &key, const size_t keylen, const std::string &after, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags)
{
    return writeKey(key, keylen, after, after.size(), cursor, maxSize, flags);
}

unsigned int CurseOutput::write(const int &x, const int &y, JSonElement *item, unsigned int maxWidth, OutputFlag flags)
{
    return write(x, y, item->stringify(), item->lazystrlen(), maxWidth, flags);
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

