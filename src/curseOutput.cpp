#include <iostream>

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#include "curseOutput.hh"
#include "jsonObject.hh"

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

    if (dynamic_cast<const JSonObject *>(item))
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
        if (!writeContent(cursor, maxSize, (const std::list<JSonElement *> *)item))
            return false;
        cursor.second += write(cursor.first, cursor.second, childDelimiter[1], maxSize.first, selection == item);
    }
    cursor.first -= indentLevel /2;
    return (cursor.second - topleft <= maxSize.second -1);
}

bool CurseOutput::writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const std::list<JSonElement*> *_item)
{
    const JSonContainer *item = (const JSonContainer *)_item;
    bool containerIsObject = (dynamic_cast<const JSonObject *>(item) != nullptr);
    cursor.first += indentLevel /2;
    for (std::list<JSonElement *>::const_iterator i = item->cbegin(); i != item->cend(); ++i)
    {
        bool isObject = (dynamic_cast<const JSonObject *>(*i) != nullptr);
        if (containerIsObject)
        {
            JSonObjectEntry *ent = (JSonObjectEntry*) *i;
            std::string key = ent->stringify();
            checkSelection(**ent, (JSonContainer*) item, cursor);
            if (collapsed.find((const JSonContainer*)(**ent)) != collapsed.cend())
            {
                if (isObject)
                    cursor.second += write(cursor.first, cursor.second, key + ": { ... }", maxSize.first, selection == **ent);
                else
                    cursor.second += write(cursor.first, cursor.second, key + ": [ ... ]", maxSize.first, selection == **ent);
            }
            else if (dynamic_cast<const JSonContainer*> (**ent) == nullptr)
                cursor.second += write(cursor.first, cursor.second, key + ": " +((**ent)->stringify()), maxSize.first, selection == **ent);
            else if (((JSonContainer*)(**ent))->size() == 0)
            {
                if (isObject)
                    cursor.second += write(cursor.first, cursor.second, key + ": { }", maxSize.first, selection == **ent);
                else
                    cursor.second += write(cursor.first, cursor.second, key + ": [ ]", maxSize.first, selection == **ent);
            }
            else
            {
                if (!writeKey(key, cursor, maxSize, selection == ent || selection == **ent))
                    return false;
                cursor.first -= indentLevel /2;
                if (!redraw(cursor, maxSize, **ent, (const JSonContainer *)item))
                    return false;
                cursor.first -= indentLevel /2;
            }
        }
        else
        {
            if (!redraw(cursor, maxSize, *i, (const JSonContainer *)item))
                return false;
        }
    }
    cursor.first -= indentLevel /2;
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

const JSonElement *CurseOutput::findPrev(const JSonElement *item)
{
    const JSonContainer *parent = item->getParent();
    if (parent == nullptr)
        return nullptr; // Root node, can't have brothers
    std::list<JSonElement *>::const_iterator it = parent->cbegin();
    const JSonObjectEntry *ent = dynamic_cast<const JSonObjectEntry *>(*it);
    const JSonElement *prevElem = ent ? **ent : (*it);
    if (prevElem == item || (ent && **ent == item))
        return nullptr; // First item
    while ((++it) != parent->cend())
    {
        ent = dynamic_cast<const JSonObjectEntry *>(*it);
        if (*it == item || (ent && **ent == item))
            return prevElem;
        prevElem = ent ? **ent : (*it);
    }
    return nullptr;
}

const JSonElement* CurseOutput::findNext(const JSonElement *item)
{
    const JSonContainer *parent = item->getParent();
    if (parent == nullptr)
        return nullptr; // Root node, can't have brothers
    JSonContainer::const_iterator it = parent->cbegin();
    while (it != parent->cend())
    {
        const JSonObjectEntry *ent = dynamic_cast<const JSonObjectEntry *>(*it);
        if (*it == item || (ent && **ent == item))
        {
            it++;
            if (it == parent->cend())
                return findNext((const JSonElement*) parent); // Last item
            ent = dynamic_cast<const JSonObjectEntry *>(*it);
            if (ent)
                return **ent;
            return *it;
        }
        it++;
    }
    return findNext((const JSonElement*) parent);
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
                const JSonElement *brother = findPrev(selection);
                if (brother == nullptr)
                {
                    const JSonContainer *parent = selection->getParent();
                    if (parent)
                        selection = parent;
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
                const JSonElement *brother = findNext(selection);
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

