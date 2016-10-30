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
    struct winsize size;

    runningInst = nullptr;
    if (ioctl(fileno(screen_fd ? screen_fd : stdout), TIOCGWINSZ, &size) == 0)
        resize_term(size.ws_row, size.ws_col);
}

void CurseOutput::loop(WINDOW * w)
{
    inputResult read;

    breakLoop = false;
    do
    {
        while (!redraw());
        read = readInput(w);
    } while (read != inputResult::quit);
}

void CurseOutput::onResizeHandler()
{
    clear();
}

bool CurseOutput::onsig(int signo)
{
    struct winsize size;
    t_Cursor oldScrSize;

    switch (signo)
    {
    case SIGWINCH:
        if (ioctl(fileno(screen_fd ? screen_fd : stdout), TIOCGWINSZ, &size) == 0)
            resize_term(size.ws_row, size.ws_col);
        screenSize = getScreenSize();
        onResizeHandler();
        while (!redraw());
        break;

    case SIGCONT:
        oldScrSize = getScreenSizeUnsafe();
        if (ioctl(fileno(screen_fd ? screen_fd : stdout), TIOCGWINSZ, &size) == -1)
            break;
        if (size.ws_row != oldScrSize.second || size.ws_col != oldScrSize.first)
            kill(getpid(), SIGWINCH);
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
    if (runningInst)
        runningInst->onsig(signo);
    signal(signo, _resizeFnc);
}

/**
 * Read input and expect signal
 * @Return true on:
 *  - Windows resized
 *  - Key press and need redraw
 * false on:
 *  - exit signal
**/
inputResult CurseOutput::readInput(WINDOW *w)
{
    while (!breakLoop)
    {
        inputResult r = evalKey(InputSequence::read(w));
        if (r != inputResult::nextInput)
            return r;
    }
    return inputResult::quit;
}

inputResult CurseOutput::evalKey(const InputSequence &c)
{
    const std::string key = c.key();

    if (key == "Q")
        return inputResult::quit;

    else if (key == "K" || key == "KEY_UP")
        return selectUp();

    else if (key == "J" || key == "KEY_DOWN")
        return selectDown();

    else if (key == "KEY_PPAGE")
        return selectPUp();

    else if (key == "KEY_NPAGE")
        return selectPDown();

    else if (key == "L" || key == "KEY_RIGHT")
        return expandSelection();

    else if (key == "H" || key == "KEY_LEFT")
        return collapseSelection();

    else if (key == "/")
        return initSearch();

    else if (key == "N")
        return nextResult();

    else if (key == "^W-W")
        return changeWindow(1, true);
    else if (key == "^W-KEY_RIGHT" || key == "^W-L")
        return changeWindow(1, false);
    else if (key == "^W-KEY_LEFT" || key == "^W-H")
        return changeWindow(-1, false);

    return inputResult::nextInput;
}

bool CurseOutput::redraw(const std::string &errorMsg)
{
    bool result = redraw();

    writeBottomLine(errorMsg, OutputFlag::SPECIAL_ERROR);
    return result;
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

bool CurseOutput::hasReachedBottom(unsigned int pos, unsigned int scrollTop, unsigned int height) const
{
    return (pos >= scrollTop && (pos -scrollTop) > height -1);
}

const t_Cursor CurseOutput::getScreenSizeUnsafe() const
{
    t_Cursor bs;
    t_Cursor sc;
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

const SearchPattern *CurseOutput::inputSearch(WINDOW *w)
{
    std::wstring buffer;
    bool abort = false;
    if (!w)
        w = stdscr;

    curs_set(true);
    wtimeout(w, -1);
    while (!abort)
    {
        int c;

        writeBottomLine(L'/' +buffer, OutputFlag::SPECIAL_SEARCH);
        c = wgetch(w);
        if (c == L'\n' || c == L'\r')
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
    wtimeout(w, 150);
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

void CurseOutput::writeTopLine(const std::string &buffer, short color) const
{
    const std::string str = buffer.substr(0, screenSize.first);
    const size_t bufsize = str.size();

    if (params.colorEnabled())
        attron(COLOR_PAIR(color));
    mvprintw(0, 0, "%s%*c", str.c_str(), screenSize.first - bufsize, ' ');
    if (params.colorEnabled())
        attroff(COLOR_PAIR(color));
}

void CurseOutput::writeBottomLine(const std::string &buffer, short color) const
{
    const t_Cursor screenSize = getScreenSizeUnsafe();
    const size_t bufsize = buffer.size();

    if (params.colorEnabled())
        attron(COLOR_PAIR(color));
    mvprintw(screenSize.second -1, 0, "%s%*c", buffer.c_str(), screenSize.first - bufsize, ' ');
    move(screenSize.second -1, bufsize);
    if (params.colorEnabled())
        attroff(COLOR_PAIR(color));
    refresh();
}

void CurseOutput::writeBottomLine(const std::wstring &buffer, short color) const
{
    const t_Cursor screenSize = getScreenSizeUnsafe();
    const size_t bufsize = buffer.size();

    if (params.colorEnabled())
        attron(COLOR_PAIR(color));
    mvprintw(screenSize.second -1, 0, "%S%*c", buffer.c_str(), screenSize.first - bufsize, ' ');
    move(screenSize.second -1, bufsize);
    if (params.colorEnabled())
        attroff(COLOR_PAIR(color));
    refresh();
}

void CurseOutput::init()
{
    if (!isatty(fileno(stdin)) || !isatty(fileno(stdout))) //TODO remove after v1.2 and #29
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
    screenSize = getScreenSize();

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
        init_pair(OutputFlag::SPECIAL_ACTIVEINPUTNAME, COLOR_BLACK, COLOR_GREEN);
        init_pair(OutputFlag::SPECIAL_INPUTNAME, COLOR_BLACK, COLOR_WHITE);

        init_pair(OutputFlag::DIFF_ADD, COLOR_GREEN, COLOR_BLACK);
        init_pair(OutputFlag::DIFF_MOD, COLOR_CYAN, COLOR_BLACK);
        init_pair(OutputFlag::DIFF_REM, COLOR_MAGENTA, COLOR_BLACK);

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
    signal(SIGCONT, _resizeFnc);
}

