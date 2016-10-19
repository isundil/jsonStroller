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

template<class T> const T &list_at(const std::list<T> &l, unsigned int pos)
{
    typename std::list<T>::const_iterator it = l.cbegin();
    std::advance(it, pos);
    return *it;
}

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
        subwin.selection = subwin.lastSelection = subwin.root = roots.at(i);
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
    setSelection(subWindows.at(selectedWin).select_up);
    return inputResult::redraw;
}

inputResult CurseSplitOutput::selectDown()
{
    const JSonElement *newSelection = subWindows.at(selectedWin).select_down;
    size_t i = 0;

    for (t_subWindow &w : subWindows)
    {
        if (w.selectIsLast)
            w.scrollTop += 2;
        if (i == selectedWin)
            w.selection = w.lastSelection = newSelection;
        else
        {
            w.selection = diffMatrice->getEquivalence(newSelection);
            if (w.selection)
                w.lastSelection = w.selection;
        }
        ++i;
    }
    return inputResult::redraw;
}

void CurseSplitOutput::setSelection(const JSonElement *selection)
{
    size_t i =0;

    for (t_subWindow &w : subWindows)
    {
        if (i == selectedWin)
            w.selection = w.lastSelection = selection;
        else
        {
            w.selection = diffMatrice->getEquivalence(selection);
            if (w.selection)
                w.lastSelection = w.selection;
        }
        ++i;
    }
}

inputResult CurseSplitOutput::selectPUp()
{
    const JSonElement *_selection = subWindows.at(selectedWin).selection;
    const JSonElement *nextSelection = _selection->findPrev();

    if (nextSelection == nullptr)
    {
        const JSonElement *parent = _selection->getParent();

        if (parent && dynamic_cast<const JSonContainer*>(parent))
        {
            nextSelection = _selection = parent;
            if (_selection->getParent() && dynamic_cast<const JSonObjectEntry*> (_selection->getParent()))
                nextSelection = _selection->getParent();
        }
        else
            return inputResult::nextInput;
    }
    setSelection(nextSelection);
    return inputResult::redraw;
}

inputResult CurseSplitOutput::selectPDown()
{
    const JSonElement *brother = subWindows.at(selectedWin).selection->findNext();

    if (brother)
    {
        setSelection(brother);
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
    {
        _selection = diffMatrice->getEquivalence(_selection);
        collapsed.erase((const JSonContainer *)_selection);
        return inputResult::redraw;
    }
    if (!((const JSonContainer*)_selection)->size())
        return inputResult::nextInput;
    selectDown();
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
        _selection = subWindows.at(selectedWin).selection->getParent();
        if (_selection->getParent() && dynamic_cast<const JSonObjectEntry*>(_selection->getParent()))
            setSelection(_selection->getParent());
        else
            setSelection(_selection);
    }
    else
    {
        collapsed.insert((const JSonContainer *)_selection);
        _selection = diffMatrice->getEquivalence(_selection);
        if (_selection)
            collapsed.insert((const JSonContainer *)_selection);
    }
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
    t_subWindow &w = subWindows.at(selectedWin);
    if (!w.selection)
    {
        size_t i =0;
        for (t_subWindow &it : subWindows)
        {
            if (i == selectedWin)
                it.selection = it.lastSelection;
            else
            {
                it.selection = diffMatrice->getEquivalence(w.lastSelection);
                if (it.selection)
                    it.lastSelection = it.selection;
            }
            ++i;
        }
    }
    return inputResult::redrawAll;
}

void CurseSplitOutput::checkSelection(const JSonElement *item)
{
    t_subWindow &w = subWindows.at(workingWin);

    if (!w.selectFound)
    {
        if (w.lastSelection == item)
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
                w.lastSelection != parent &&
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

    if (subWindows.at(selectedWin).lastSelection == current)
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
            subWindows.at(selectedWin).lastSelection = current;
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
        subWindows.at(selectedWin).lastSelection = *(subWindows.at(selectedWin).searchResults.cbegin());
        unfold(subWindows.at(selectedWin).selection);
        CurseOutput::redraw("Search hit BOTTOM, continuing at TOP");
        return false;
    }
    unfold(subWindows.at(selectedWin).selection);
    return true;
}

const Optional<bool> CurseSplitOutput::redrawOneItemToWorkingWin(t_subWindow &w, const t_Cursor &screenSize)
{
    bool result;

    try {
        if (w.parentsIterators.empty())
            result = redraw(screenSize, w.root);
        else
            result = redraw(screenSize, w.parentsIterators.top());
    }
    catch (SelectionOutOfRange &e)
    {
        return Optional<bool>::empty;
    }
    catch (CurseSplitOutput::reachNext &)
    {
        result = true;
    }
    if (!result || w.parentsIterators.empty())
    {
        if (!result)
        {
            if (!w.selectFound)
            {
                w.scrollTop++;
                return Optional<bool>::empty;
            }
            if (!w.select_down)
                w.selectIsLast = true;
        }
        if (!w.select_down)
        {
            const JSonContainer *pselect = dynamic_cast<const JSonContainer*>(w.selection);
            if (pselect && !pselect->empty() && collapsed.find(pselect) == collapsed.cend())
                w.select_down = *(pselect->cbegin());
            else
            {
                const JSonElement *next = w.lastSelection->findNext();
                w.select_down = next ? next : w.lastSelection;
            }
        }
        if (!w.select_up)
            w.select_up = subWindows.at(workingWin).lastSelection;
        return Optional<bool>::of(true);
    }
    return Optional<bool>::of(false);
}

bool CurseSplitOutput::redraw()
{
    const t_Cursor screenSize = getScreenSize();
    short writingDone = (1 << nbInputs) -1;

    workingWin = 0;
    for (t_subWindow &w : subWindows)
    {
        w.cursor = t_Cursor(2, 1);
        w.select_up = w.select_down = nullptr;
        w.selectFound = w.selectIsLast = false;

        wclear(w.innerWin);
        writeTopLine(w.fileName,
                workingWin == selectedWin ? OutputFlag::SPECIAL_ACTIVEINPUTNAME : OutputFlag::SPECIAL_INPUTNAME);
        w.parentsIterators = std::stack<std::pair<int, JSonContainer*> >();
        ++workingWin;
    }
    while (writingDone)
    {
        // Display Gap (--)
        bool restart = false;
        workingWin = 0;
        for (t_subWindow &w : subWindows)
        {
            if ((writingDone & (1 << workingWin)) &&
                    ((!w.parentsIterators.empty() && isAdded(w.parentsIterators.top())) ||
                     (w.parentsIterators.empty() && isAdded(w.root))))
            {
                const unsigned int startY = w.cursor.second;

                do
                {
                    const Optional<bool> wrote = redrawOneItemToWorkingWin(w, screenSize);

                    if (wrote.absent())
                        return false;
                    if (wrote.get())
                    {
                        writingDone &= ~(1 << workingWin);
                        break;
                    }
                } while (isAdded(w.parentsIterators.top()));

                const unsigned int diffY = w.cursor.second - startY;
                unsigned int i = 0;

                for (t_subWindow &wi: subWindows)
                    if (i++ != workingWin)
                        for (unsigned int j = 0; j < diffY; ++j)
                            displayDiffOp(wi.innerWin, (wi.cursor.second)++, eLevenshteinOperator::rem);
                restart = true;

                break;
            }
            ++workingWin;
        }
        if (restart)
            continue;

        // Actual display
        workingWin = 0;
        for (t_subWindow &w : subWindows)
        {
            if ((writingDone & (1 << workingWin)))
            {
                const Optional<bool> wrote = redrawOneItemToWorkingWin(w, screenSize);

                if (wrote.absent())
                    return false;
                if (wrote.get())
                    writingDone &= ~(1 << workingWin);
            }
            ++workingWin;
        }
    }
    for (t_subWindow &w : subWindows)
        wrefresh(w.innerWin);
    return true;
}

bool CurseSplitOutput::writeContainer(const t_Cursor &maxSize, JSonContainer *item, bool opening)
{
    char childDelimiter[2];
    t_subWindow &w = subWindows.at(workingWin);

    if (dynamic_cast<const JSonObject *>(item))
        memcpy(childDelimiter, "{}", sizeof(*childDelimiter) * 2);
    else
        memcpy(childDelimiter, "[]", sizeof(*childDelimiter) * 2);

    if (!opening)
    {
        w.cursor.first -= INDENT_LEVEL;
        w.cursor.second += write(w.cursor.first, w.cursor.second, childDelimiter[1], maxSize.first, CurseSplitOutput::getFlag(item));
    }
    else if (collapsed.find((const JSonContainer *)item) != collapsed.end())
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
        w.parentsIterators.push(std::pair<int, JSonContainer *>(-1, item));
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
                const JSonElement *saveSelection = w.lastSelection;
                if (saveSelection == ent)
                    w.selection = w.lastSelection = **ent;
                w.cursor.first += INDENT_LEVEL /2;
                throw CurseSplitOutput::reachNext();
            }
        }
        else
            throw CurseSplitOutput::reachNext();
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

bool CurseSplitOutput::isAdded(const JSonElement *e) const
{
    return diffMatrice->getEquivalence(e) == nullptr;
}

bool CurseSplitOutput::isAdded(const std::pair<int, JSonContainer *> &item) const
{
    const JSonElement *e;

    if ((unsigned int) (item.first +1) >= item.second->size())
        e = item.second;
    else
        e = list_at<JSonElement*>(*(item.second), item.first +1);
    return isAdded(e);
}

bool CurseSplitOutput::redraw(const t_Cursor &maxSize, std::pair<int, JSonContainer *> &item)
{
    t_subWindow &w = subWindows.at(workingWin);
    JSonElement *currentItem;

    (item.first)++;
    if ((unsigned int) item.first == item.second->size())
    {
        w.parentsIterators.pop();
        if (!writeContainer(maxSize, item.second, false))
            return false;
        throw CurseSplitOutput::reachNext();
    }
    currentItem = list_at<JSonElement*>(*(item.second), item.first);
    checkSelection(currentItem);
    if (dynamic_cast<const JSonContainer*>(currentItem))
    {
        if (!writeContainer(maxSize, (JSonContainer*) currentItem))
            return false;
    }
    else
    {
        w.cursor.second += CurseOutput::write(w.cursor.first, w.cursor.second, currentItem, maxSize.first, CurseSplitOutput::getFlag(currentItem));
        if (w.cursor.second > w.scrollTop && (w.cursor.second - w.scrollTop) > maxSize.second -1)
            return false;
    }
    return true;
}

bool CurseSplitOutput::redraw(const t_Cursor &maxSize, JSonElement *item)
{
    checkSelection(item);
    if (dynamic_cast<const JSonContainer*>(item))
    {
        if (!writeContainer(maxSize, (JSonContainer *) item))
            return false;
    }
    else
    {
        t_subWindow &w = subWindows.at(workingWin);

        w.cursor.second += CurseOutput::write(w.cursor.first, w.cursor.second, item, maxSize.first, CurseSplitOutput::getFlag(item));
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

    displayDiffOp(currentWin, offsetY, flags.diffOp());
    return getNbLines(x +1, maxWidth);
}

void CurseSplitOutput::displayDiffOp(WINDOW *w, const int &y, const eLevenshteinOperator &op) const
{
    switch (op)
    {
        case eLevenshteinOperator::add:
            wattron(w, A_REVERSE | A_BOLD);
            wattron(w, COLOR_PAIR(OutputFlag::DIFF_ADD));
            mvwprintw(w, y, 0, "++");
            wattroff(w, COLOR_PAIR(OutputFlag::DIFF_ADD));
            wattroff(w, A_REVERSE | A_BOLD);
            break;

        case eLevenshteinOperator::mod:
            wattron(w, A_REVERSE | A_BOLD);
            wattron(w, COLOR_PAIR(OutputFlag::DIFF_MOD));
            mvwprintw(w, y, 0, "!!");
            wattroff(w, COLOR_PAIR(OutputFlag::DIFF_MOD));
            wattroff(w, A_REVERSE | A_BOLD);
            break;

        case eLevenshteinOperator::rem:
            wattron(w, A_REVERSE | A_BOLD);
            wattron(w, COLOR_PAIR(OutputFlag::DIFF_REM));
            mvwprintw(w, y, 0, "--");
            wattroff(w, COLOR_PAIR(OutputFlag::DIFF_REM));
            wattroff(w, A_REVERSE | A_BOLD);
            break;

        case eLevenshteinOperator::equ: // skip
            break;
    }
}

unsigned int CurseSplitOutput::write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags)
{
    const t_subWindow &w = subWindows.at(workingWin);
    WINDOW *currentWin = w.innerWin;
    int offsetY = y - w.scrollTop;

    if (offsetY <= 0)
        return 1;
    displayDiffOp(currentWin, y, flags.diffOp());
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
        res.diffOp(diffMatrice->get(item));
    }
    catch (std::out_of_range &e) {
        res.diffOp(eLevenshteinOperator::add);
    }

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
    return res;
}

