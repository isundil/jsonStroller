/**
 * curseSplitOutput.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <curses.h>

#include "curseSplitOutput.hh"
#include "jsonContainer.hh"

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
    selection = data = *roots.begin();
    nbInputs = inputName.size();
    currentWin = 0;
    for (size_t i =0; i < nbInputs; i++)
    {
        this->roots.push_back(roots.at(i));
        selections.push_back(roots.at(i));
    }
    fileNames = inputName;
    loop();
}

bool CurseSplitOutput::redraw()
{
    const std::pair<unsigned int, unsigned int> screenSize = getScreenSize();

    destroyAllSubWin();
    for (size_t i=0; i < nbInputs; i++)
    {
        WINDOW *currentWin = newwin(screenSize.second, screenSize.first, 0, i * screenSize.first - (i ? 1 : 0));
        box(currentWin, 0, 0);
        redraw(currentWin, screenSize); // TODO
        wrefresh(currentWin);
    }
    refresh();
    return false;

    // TODO
    std::pair<int, int> cursor(0, 0);
    /**
     * Will be true if the json's last item is visible
    **/
    bool result;

    select_up = select_down = nullptr;
    selectFound = selectIsLast = false;
    clear();
    try {
        result = CurseOutput::redraw(cursor, screenSize, data);
    }
    catch (SelectionOutOfRange &e)
    {
        return false;
    }
    if (!result && !selectFound)
    {
        scrollTop++;
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
    scrollTop = 0;
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

