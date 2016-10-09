/**
 * curseSplitOutput.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <curses.h>

#include "searchPattern.hh"
#include "curseSplitOutput.hh"
#include "jsonObject.hh"
#include "jsonArray.hh"
#include "jsonPrimitive.hh"
#include "levenshteinMatrice.hpp"

CurseSplitOutput::CurseSplitOutput(const Params &p): CurseOutput(p)
{
    diffMatrice = nullptr;
    init();
}

CurseSplitOutput::~CurseSplitOutput()
{
    shutdown();
    if (diffMatrice)
    {
        delete diffMatrice;
        diffMatrice = nullptr;
    }
}

void CurseSplitOutput::run(const std::deque<std::string> &inputName, const std::deque<JSonElement*> &roots)
{
    nbInputs = inputName.size();
    const t_Cursor screenSize = getScreenSize();
    selectedWin = 0;
    destroyAllSubWin();
    subWindows.clear();

    for (size_t i =0; i < nbInputs; ++i)
    {
        t_subWindow subwin;

        subwin.fileName = inputName.at(i);
        subwin.selection = subwin.root = roots.at(i);
        subwin.select_up = subwin.select_down = nullptr;
        subwin.innerWin = subwin.outerWin = nullptr;
        subwin.scrollTop = 0;
        subwin.outerWin = newwin(screenSize.second +2, screenSize.first, 0, i * (screenSize.first -1));
        subwin.innerWin = newwin(screenSize.second, screenSize.first -2, 1, i * (screenSize.first -1) +1);
        keypad(subwin.outerWin, true);
        subWindows.push_back(subwin);
        box(subwin.outerWin, 0, 0);
        wrefresh(subwin.outerWin);
    }
    computeDiff();
    loop(subWindows.at(0).outerWin);
}

void CurseSplitOutput::computeDiff()
{
    //TODO diffMatrice should be LevenshteinMatrice_base[nbInputs -1]
    //And we should iterate such as diffMatrice[n] = diff(n, n+1) ?

    if (diffMatrice)
        delete diffMatrice;
    LevenshteinMatrice_base::Builder builder;
    if (nbInputs == 2)
        diffMatrice = builder.build(subWindows.at(0).root, subWindows.at(1).root);
    else if (nbInputs == 3)
        throw std::runtime_error("3-input diff not implemented");
}

inputResult CurseSplitOutput::selectUp()
{
    subWindows.at(selectedWin).selection = subWindows.at(selectedWin).select_up;
    return inputResult::redraw;
}

inputResult CurseSplitOutput::selectDown()
{
    t_subWindow &w = subWindows.at(selectedWin);

    if (w.selectIsLast)
        w.scrollTop += 2;
    else if (w.selection != w.select_down)
        w.selection = w.select_down;
    else
        return inputResult::nextInput;
    return inputResult::redraw;
}

inputResult CurseSplitOutput::selectPUp()
{
    const JSonElement *_selection = subWindows.at(selectedWin).selection;
    const JSonElement *brother = _selection->findPrev();

    if (brother == nullptr)
    {
        const JSonElement *parent = _selection->getParent();
        if (parent && dynamic_cast<const JSonContainer*>(parent))
        {
            subWindows.at(selectedWin).selection = _selection = parent;
            if (_selection->getParent() && dynamic_cast<const JSonObjectEntry*> (_selection->getParent()))
                subWindows.at(selectedWin).selection = _selection->getParent();
        }
        else
            return inputResult::nextInput;
    }
    else
        subWindows.at(selectedWin).selection = brother;
    return inputResult::redraw;
}

inputResult CurseSplitOutput::selectPDown()
{
    const JSonElement *brother = subWindows.at(selectedWin).selection->findNext();

    if (brother)
    {
        subWindows.at(selectedWin).selection = brother;
        return inputResult::redraw;
    }
    return inputResult::nextInput;
}

inputResult CurseSplitOutput::expandSelection()
{
    const JSonElement *_selection = subWindows.at(selectedWin).selection;

    if (dynamic_cast<const JSonObjectEntry*>(_selection))
        _selection = **((const JSonObjectEntry*)_selection);
    if (!dynamic_cast<const JSonContainer*>(_selection))
        return inputResult::nextInput;

    if (collapsed.erase((const JSonContainer *)_selection))
        return inputResult::redraw;
    if (!((const JSonContainer*)_selection)->size())
        return inputResult::nextInput;
    subWindows.at(selectedWin).selection = subWindows.at(selectedWin).select_down;
    return inputResult::redraw;
}

inputResult CurseSplitOutput::collapseSelection()
{
    const JSonElement *_selection = subWindows.at(selectedWin).selection;

    if (dynamic_cast<const JSonObjectEntry*>(_selection))
        _selection = **((const JSonObjectEntry*)_selection);
    if (_selection->getParent() && (!dynamic_cast<const JSonContainer*>(_selection)
            || collapsed.find((const JSonContainer *)_selection) != collapsed.end()
            || (dynamic_cast<const JSonContainer*>(_selection) && ((const JSonContainer*)_selection)->size() == 0)))
    {
        subWindows.at(selectedWin).selection = _selection = subWindows.at(selectedWin).selection->getParent();
        if (_selection->getParent() && dynamic_cast<const JSonObjectEntry*>(_selection->getParent()))
            subWindows.at(selectedWin).selection = _selection->getParent();
    }
    else
        collapsed.insert((const JSonContainer *)_selection);
    return inputResult::redraw;
}

inputResult CurseSplitOutput::initSearch()
{
    const SearchPattern *searchPattern = inputSearch();
    if (!searchPattern)
        return inputResult::redraw;
    for (t_subWindow &s : subWindows)
        s.searchResults.clear();
    if (searchPattern->isEmpty())
        return inputResult::redraw;
    search(*searchPattern);
    delete searchPattern;
    return nextResult();
}

inputResult CurseSplitOutput::nextResult()
{
    if (subWindows.at(selectedWin).searchResults.empty())
        CurseOutput::redraw("Pattern not found");
    else if (jumpToNextSearch())
        return inputResult::redraw;
    return inputResult::nextInput;
}

inputResult CurseSplitOutput::changeWindow(char d, bool c)
{
    if ((selectedWin +d < 0 || selectedWin +d >= nbInputs) && !c)
        return inputResult::nextInput;
    selectedWin = (selectedWin +d) % nbInputs;
    return inputResult::redrawAll;
}

void CurseSplitOutput::checkSelection(const JSonElement *item)
{
    t_subWindow &w = subWindows.at(workingWin);

    if (!w.selectFound)
    {
        if (w.selection == item)
        {
            if (w.cursor.second < w.scrollTop) //Selection is above vp, move scroll pos to selection and start drawing
                w.scrollTop = w.cursor.second;
            w.selectFound = true;
        }
        else if (!item->getParent() || !dynamic_cast<const JSonObjectEntry*>(item->getParent()))
            w.select_up = item;
    }
    else if (!w.select_down)
    {
        const JSonElement *parent = item->getParent();
        if (!dynamic_cast<const JSonContainer*>(item) &&
                parent &&
                w.selection != parent &&
                dynamic_cast<const JSonObjectEntry*>(parent))
            item = parent;
        if (!parent || !dynamic_cast<const JSonObjectEntry*>(parent))
            w.select_down = item;
    }
}

bool CurseSplitOutput::jumpToNextSearch(const JSonElement *current, bool &selectFound)
{
    const JSonContainer *container = dynamic_cast<const JSonContainer *> (current);
    const JSonObjectEntry *objEntry = dynamic_cast<const JSonObjectEntry *> (current);

    if (subWindows.at(selectedWin).selection == current)
        selectFound = true;
    if (container)
    {
        if (!container->empty())
            for (const JSonElement *it : *container)
                if (jumpToNextSearch(it, selectFound))
                    return true;
    }
    else
    {
        if (current &&
                std::find(subWindows.at(selectedWin).searchResults.cbegin(),
                    subWindows.at(selectedWin).searchResults.cend(),
                    current) != subWindows.at(selectedWin).searchResults.cend() &&
                current != subWindows.at(selectedWin).selection &&
                selectFound)
        {
            subWindows.at(selectedWin).selection = current;
            return true;
        }
        if (objEntry)
            if (jumpToNextSearch(**objEntry, selectFound))
                return true;
    }
    return false;
}

unsigned int CurseSplitOutput::search(const SearchPattern &searchPattern)
{
    unsigned int result =0;

    for (t_subWindow &w : subWindows)
        result += search(searchPattern, w.root);
    return result;
}

unsigned int CurseSplitOutput::search(const SearchPattern &searchPattern, const JSonElement *current)
{
    const JSonContainer *container = dynamic_cast<const JSonContainer *> (current);
    const JSonObjectEntry *objEntry = dynamic_cast<const JSonObjectEntry *> (current);

    if (container)
    {
        if (!container->empty())
            for (const JSonElement *it : *container)
                search(searchPattern, it);
    }
    else
    {
        if (current && current->match(searchPattern))
        {
            if (current->getParent() && dynamic_cast<const JSonObjectEntry*>(current->getParent()))
                subWindows.at(workingWin).searchResults.push_back(current->getParent());
            else
                subWindows.at(workingWin).searchResults.push_back(current);
        }
        if (objEntry)
            search(searchPattern, **objEntry);
    }
    return subWindows.at(workingWin).searchResults.size();
}

bool CurseSplitOutput::jumpToNextSearch()
{
    bool selectFound = false;
    bool res = jumpToNextSearch(subWindows.at(selectedWin).root, selectFound);

    if (!res)
    {
        subWindows.at(selectedWin).selection = *(subWindows.at(selectedWin).searchResults.cbegin());
        unfold(subWindows.at(selectedWin).selection);
        CurseOutput::redraw("Search hit BOTTOM, continuing at TOP");
        return false;
    }
    unfold(subWindows.at(selectedWin).selection);
    return true;
}

bool CurseSplitOutput::redraw()
{
    const t_Cursor screenSize = getScreenSize();
    short writingDone = (1 << nbInputs) -1;

    workingWin = 0;
    for (t_subWindow &w : subWindows)
    {
        w.cursor = t_Cursor(0, 1);
        w.select_up = w.select_down = nullptr;
        w.selectFound = w.selectIsLast = false;

        wclear(w.innerWin);
        writeTopLine(w.fileName,
                workingWin == selectedWin ? OutputFlag::SPECIAL_ACTIVEINPUTNAME : OutputFlag::SPECIAL_INPUTNAME);
        w.parentsIterators = std::stack<JSonContainer::const_iterator>();
        ++workingWin;
    }
    while (writingDone)
    {
        workingWin = 0;
        for (t_subWindow &w : subWindows)
        {
            bool result;

            if ((writingDone & (1 << workingWin)) == 0)
                continue;
            try {
                if (w.parentsIterators.empty())
                    result = redraw(screenSize, w.root, true);
                else
                    result = redraw(screenSize, w.parentsIterators.top(), true);
            }
            catch (SelectionOutOfRange &e)
            {
                return false;
            }
            catch (CurseSplitOutput::reachNext &)
            {
                ++workingWin;
                continue;
            }
            if (!result)
            {
                if (!w.selectFound)
                {
                    w.scrollTop++;
                    return false;
                }
                if (!w.select_down)
                    w.selectIsLast = true;
            }
            if (!w.select_down)
            {
                const JSonContainer *pselect = dynamic_cast<const JSonContainer*>(w.selection);
                if (pselect && !pselect->empty())
                    w.select_down = *(pselect->cbegin());
                else
                {
                    const JSonElement *next = w.selection->findNext();
                    w.select_down = next ? next : w.selection;
                }
            }
            if (!w.select_up)
                w.select_up = subWindows.at(workingWin).selection;
            writingDone &= ~(1 << workingWin);
            ++workingWin;
        }
    }
    for (t_subWindow &w : subWindows)
    {
        wrefresh(w.innerWin);
    }
    return true;
}

bool CurseSplitOutput::writeContainer(const t_Cursor &maxSize, JSonContainer *item)
{
    char childDelimiter[2];
    t_subWindow &w = subWindows.at(workingWin);

    if (dynamic_cast<const JSonObject *>(item))
        memcpy(childDelimiter, "{}", sizeof(*childDelimiter) * 2);
    else
        memcpy(childDelimiter, "[]", sizeof(*childDelimiter) * 2);

    if (collapsed.find((const JSonContainer *)item) != collapsed.end())
    {
        std::string ss;
        ss.append(&childDelimiter[0], 1).append(" ... ").append(&childDelimiter[1], 1);
        w.cursor.second += write(w.cursor.first, w.cursor.second, ss, 7, maxSize.first, CurseSplitOutput::getFlag(item));
    }
    else
    {
        w.cursor.second += write(w.cursor.first, w.cursor.second, childDelimiter[0], maxSize.first, CurseSplitOutput::getFlag(item));
        if (w.cursor.second > w.scrollTop && (w.cursor.second - w.scrollTop) > maxSize.second -1)
            return false;
        w.parentsIterators.push(item->cbegin());
        if (!writeContent(maxSize, (std::list<JSonElement *> *)item))
        {
            w.parentsIterators.pop();
            return false;
        }
        w.parentsIterators.pop();
        w.cursor.second += write(w.cursor.first, w.cursor.second, childDelimiter[1], maxSize.first, CurseSplitOutput::getFlag(item));
    }
    return (w.cursor.second < w.scrollTop || (w.cursor.second - w.scrollTop) <= maxSize.second -1);
}

bool CurseSplitOutput::writeContent(const t_Cursor &maxSize, std::list<JSonElement*> *_item)
{
    t_subWindow &w = subWindows.at(workingWin);
    JSonContainer *item = (JSonContainer *)_item;
    bool containerIsObject = (dynamic_cast<JSonObject *>(item) != nullptr);
    bool result = true;
    w.cursor.first += INDENT_LEVEL;
    const unsigned int scrollTop = w.scrollTop;

    for (JSonElement *i : *item)
    {
        result = false;
        if (containerIsObject)
        {
            JSonObjectEntry *ent = (JSonObjectEntry*) i;
            bool isContainer = (dynamic_cast<JSonContainer *>(**ent) != nullptr);
            std::string key = ent->stringify();
            checkSelection(ent);
            if (isContainer && collapsed.find((JSonContainer*)(**ent)) != collapsed.cend())
            {
                if (dynamic_cast<JSonObject *>(**ent))
                {
                    if (!CurseOutput::writeKey(key, ent->lazystrlen(), "{ ... }", w.cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (w.cursor.second > scrollTop && (w.cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!CurseOutput::writeKey(key, ent->lazystrlen(), "[ ... ]", w.cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (w.cursor.second > scrollTop && (w.cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (!isContainer)
            {
                JSonElement *eContent = **ent;
                if (!writeKey(key, ent->lazystrlen(), eContent->stringify(), eContent->lazystrlen(), w.cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (w.cursor.second > scrollTop && (w.cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else if (((JSonContainer*)(**ent))->size() == 0)
            {
                if (dynamic_cast<const JSonObject *>(**ent) )
                {
                    if (!CurseOutput::writeKey(key, ent->lazystrlen(), "{ }", w.cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (w.cursor.second > scrollTop && (w.cursor.second - scrollTop) > maxSize.second -1))
                        break;
                }
                else if (!CurseOutput::writeKey(key, ent->lazystrlen(), "[ ]", w.cursor, maxSize, CurseSplitOutput::getFlag(ent)) || (w.cursor.second > scrollTop && (w.cursor.second - scrollTop) > maxSize.second -1))
                    break;
            }
            else
            {
                if (!writeKey(key, ent->lazystrlen(), maxSize, getFlag(ent)))
                    break;
                const JSonElement *saveSelection = w.selection;
                if (saveSelection == ent)
                    w.selection = **ent;
                w.cursor.first += INDENT_LEVEL /2;
                if (!redraw(maxSize, **ent))
                {
                    w.selection = saveSelection;
                    w.cursor.first -= INDENT_LEVEL /2;
                    return false;
                }
                w.selection = saveSelection;
                w.cursor.first -= INDENT_LEVEL /2;
            }
        }
        else
        {
            if (!redraw(maxSize, i))
                break;
        }
        result = true;
    }
    w.cursor.first -= INDENT_LEVEL;
    //result will be false if for loop break'd at some time, true otherwise
    return result;
}

bool CurseSplitOutput::writeKey(const std::string &key, const size_t keylen, const t_Cursor &maxSize, OutputFlag flags, unsigned int extraLen)
{
    t_subWindow &w = subWindows.at(workingWin);

    if (w.cursor.second <= w.scrollTop)
    {
        w.cursor.second++;
        return true;
    }
    char oldType = flags.type();
    flags.type(OutputFlag::TYPE_OBJKEY);
    w.cursor.second += write(w.cursor.first, w.cursor.second, key, keylen, maxSize.first -extraLen -2, flags);
    flags.type(OutputFlag::TYPE_OBJ);
    write(": ", flags);
    flags.type(oldType);
    return (w.cursor.second < w.scrollTop || (w.cursor.second - w.scrollTop) <= maxSize.second);
}

bool CurseSplitOutput::writeKey(const std::string &key, const size_t keylen, const std::string &after, const size_t afterlen, t_Cursor &cursor, const t_Cursor &maxWidth, OutputFlag flags)
{
    t_subWindow &w = subWindows.at(workingWin);

    if (cursor.second <= w.scrollTop)
    {
        cursor.second++;
        return true;
    }
    char oldType = flags.type();
    flags.type(OutputFlag::TYPE_OBJKEY);
    write(cursor.first, cursor.second, key, 0, 1, flags);
    flags.type(OutputFlag::TYPE_OBJ);
    write(": ", flags);
    flags.type(oldType);
    write(after, flags);
    cursor.second += getNbLines(cursor.first +keylen +2 +afterlen, maxWidth.first);
    return (cursor.second < w.scrollTop || (cursor.second - w.scrollTop) <= maxWidth.second);
}

bool CurseSplitOutput::redraw(const t_Cursor &maxSize, JSonContainer::const_iterator item, bool isRoot)
{
    t_subWindow &w = subWindows.at(workingWin);
    static short dirty =0;
    JSonContainer *parent;
    JSonContainer::const_iterator next = item;

    if (dirty && !isRoot)
        throw CurseSplitOutput::reachNext();
    dirty = 1;
    next++;
    w.parentsIterators.pop();
    if (w.parentsIterators.size() <= 1)
        parent = (JSonContainer *) w.root;
    else
        parent = (JSonContainer *) (*(w.parentsIterators.top()));
    if (item != parent->cend())
        w.parentsIterators.push(next);
    else
    {
        //TODO close brackets
        return true;
    }
    checkSelection(*item);
    if (dynamic_cast<const JSonContainer*>(*item))
    {
        if (!writeContainer(maxSize, (JSonContainer*) (*item)))
        {
            dirty = 0;
            return false;
        }
        dirty = 0;
    }
    else
    {
        w.cursor.second += CurseOutput::write(w.cursor.first, w.cursor.second, *item, maxSize.first, CurseSplitOutput::getFlag(*item));
        dirty = 0;
        if (w.cursor.second > w.scrollTop && (w.cursor.second - w.scrollTop) > maxSize.second -1)
            return false;
    }
    return true;
}

bool CurseSplitOutput::redraw(const t_Cursor &maxSize, JSonElement *item, bool isRoot)
{
    static short dirty =0;

    if (dirty && !isRoot)
        throw CurseSplitOutput::reachNext();
    dirty = 1;
    checkSelection(item);
    if (dynamic_cast<const JSonContainer*>(item))
    {
        if (!writeContainer(maxSize, (JSonContainer *) item))
        {
            dirty = 0;
            return false;
        }
        dirty = 0;
    }
    else
    {
        t_subWindow &w = subWindows.at(workingWin);

        w.cursor.second += CurseOutput::write(w.cursor.first, w.cursor.second, item, maxSize.first, CurseSplitOutput::getFlag(item));
        dirty = 0;
        if (w.cursor.second > w.scrollTop && (w.cursor.second - w.scrollTop) > maxSize.second -1)
            return false;
    }
    return true;
}

unsigned int CurseSplitOutput::write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags)
{
    int offsetY = y - subWindows.at(workingWin).scrollTop;
    WINDOW *currentWin = subWindows.at(workingWin).innerWin;
    char color = OutputFlag::SPECIAL_NONE;

    if (offsetY <= 0)
        return 1;

    if (flags.selected())
        wattron(currentWin, A_REVERSE | A_BOLD);
    if (flags.searched())
        color = OutputFlag::SPECIAL_SEARCH;
    else if (colors.find(flags.type()) != colors.end())
        color = flags.type();

    if (color != OutputFlag::SPECIAL_NONE)
        wattron(currentWin, COLOR_PAIR(color));
    mvwprintw(currentWin, offsetY, x, "%c", item);
    wattroff(currentWin, A_REVERSE | A_BOLD);
    if (color != OutputFlag::SPECIAL_NONE)
        wattroff(currentWin, COLOR_PAIR(color));
    return getNbLines(x +1, maxWidth);
}

unsigned int CurseSplitOutput::write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags)
{
    int offsetY = y - subWindows.at(workingWin).scrollTop;

    if (offsetY <= 0)
        return 1;
    wmove(subWindows.at(workingWin).innerWin, offsetY, x);
    write(str, flags);
    return getNbLines(strlen +x, maxWidth);
}

void CurseSplitOutput::write(const std::string &str, const OutputFlag flags) const
{
    char color = OutputFlag::SPECIAL_NONE;
    WINDOW *currentWin = subWindows.at(workingWin).innerWin;

    if (flags.selected())
        wattron(currentWin, A_REVERSE | A_BOLD);
    if (flags.searched())
        color = OutputFlag::SPECIAL_SEARCH;
    else if (colors.find(flags.type()) != colors.end())
        color = flags.type();
    if (color != OutputFlag::SPECIAL_NONE)
        wattron(currentWin, COLOR_PAIR(color));

    wprintw(currentWin, "%s", str.c_str());
    wattroff(currentWin, A_REVERSE | A_BOLD);
    if (color != OutputFlag::SPECIAL_NONE)
        wattroff(currentWin, COLOR_PAIR(color));
}

void CurseSplitOutput::destroyAllSubWin()
{
    for (t_subWindow &w : subWindows)
    {
        if (w.outerWin)
        {
            wborder(w.outerWin, ' ', ' ', ' ',' ',' ',' ',' ',' ');
            wrefresh(w.outerWin);
            delwin(w.outerWin);
            w.outerWin = nullptr;
        }
        if (w.innerWin)
        {
            delwin(w.innerWin);
            w.innerWin = nullptr;
        }
    }
}

void CurseSplitOutput::writeTopLine(const std::string &buffer, short color) const
{
    const t_Cursor screenSize = getScreenSize();
    const size_t bufsize = buffer.size();
    WINDOW *currentWin = subWindows.at(workingWin).innerWin;

    if (params.colorEnabled())
        wattron(currentWin, COLOR_PAIR(color));
    mvwprintw(currentWin, 0, 0, "%s%*c", buffer.c_str(), screenSize.first - bufsize -2, ' ');
    if (params.colorEnabled())
        wattroff(currentWin, COLOR_PAIR(color));
}

const t_Cursor CurseSplitOutput::getScreenSize() const
{
    t_Cursor result = getScreenSizeUnsafe();
    return t_Cursor(result.first / nbInputs, result.second -2);
}

void CurseSplitOutput::shutdown()
{
    destroyAllSubWin();
    endwin();
    delscreen(screen);
    if (screen_fd)
    {
        fclose(screen_fd);
        screen_fd = nullptr;
    }
    screen = nullptr;
}

const OutputFlag CurseSplitOutput::getFlag(const JSonElement *e) const
{
    return getFlag(e, subWindows.at(workingWin).selection);
}

const OutputFlag CurseSplitOutput::getFlag(const JSonElement *item, const JSonElement *selection) const
{
    OutputFlag res;
    const JSonElement *i = dynamic_cast<const JSonObjectEntry*>(item) ? **((const JSonObjectEntry*)item) : item;

    res.selected(item == selection);
    res.searched(std::find(subWindows.at(selectedWin).searchResults.cbegin(),
                subWindows.at(selectedWin).searchResults.cend(),
                item) != subWindows.at(selectedWin).searchResults.cend());

    try {
        eLevenshteinOperator dr = diffMatrice->get(item);
        if (dr == eLevenshteinOperator::add)
            res.type(OutputFlag::TYPE_NUMBER);
        else if (dr == eLevenshteinOperator::rem)
            res.type(OutputFlag::TYPE_BOOL);
        else if (dr == eLevenshteinOperator::mod)
            res.type(OutputFlag::TYPE_STRING);
    }
    catch (std::out_of_range &e) {
        res.type(OutputFlag::SPECIAL_SEARCH);
    }
    /*
    if (dynamic_cast<const JSonPrimitive<std::string> *>(i))
        res.type(OutputFlag::TYPE_STRING);
    else if (dynamic_cast<const JSonPrimitive<bool> *>(i))
        res.type(OutputFlag::TYPE_BOOL);
    else if (dynamic_cast<const JSonPrimitive<Null> *>(i))
        res.type(OutputFlag::TYPE_NULL);
    else if (dynamic_cast<const AJSonPrimitive *>(i))
        res.type(OutputFlag::TYPE_NUMBER);
    else if (dynamic_cast<const JSonObject*>(i))
        res.type(OutputFlag::TYPE_OBJ);
    else if (dynamic_cast<const JSonArray*>(i))
        res.type(OutputFlag::TYPE_ARR);
    */
    return res;
}

