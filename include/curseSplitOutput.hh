#pragma once

#include <deque>
#include <stack>
#include "curseOutput.hh"
#include "levenshtein.hpp"
#include "jsonContainer.hh"

class LevenshteinMatrice_base;
class JSonObjectEntry;

typedef struct
{
    std::string fileName;
    JSonElement *root;
    const JSonElement *selection, *lastSelection, *select_up, *select_down;
    WINDOW *innerWin, *outerWin;
    std::list<const JSonElement*> searchResults;
    unsigned int scrollTop;
    t_Cursor cursor;
    std::stack<std::pair<int, JSonContainer*> > parentsIterators; // Offset in JSonContainer::list
    //TODO back to std::stack<JSonContainer::const_iterator> ?
    bool selectFound, selectIsLast;
} t_subWindow;

class CurseSplitOutput: public CurseOutput
{
    public:
        CurseSplitOutput(const Params &);
        virtual ~CurseSplitOutput();

        /**
         * Display data, and shutdown ncurses at the end
        **/
        void run(const std::deque<std::string> &, const std::deque<JSonElement *> &);

        void checkSelection(const JSonElement *item);

        bool redraw();
        bool redraw(t_subWindow &, std::pair<int, JSonContainer *> &);
        bool redraw(t_subWindow &, JSonElement *);
        const Optional<bool> redrawOneItemToWorkingWin(t_subWindow &w);
        bool isAdded(const std::pair<int, JSonContainer *> &) const;
        bool isAdded(const JSonElement *e) const;

        bool writeContainer(JSonContainer *, bool opening = true);
        bool writeObjectEntry(t_subWindow &w, JSonObjectEntry *item);
        bool writeKey(t_subWindow &, const std::string &key, const size_t keylen, OutputFlag flags, unsigned int extraLen =0);
        bool writeKey(t_subWindow &, const std::string &key, const size_t keylen, const std::string &after, const size_t afterlen, t_Cursor &cursor, OutputFlag);
        bool writeKey(t_subWindow &, const std::string &key, const size_t keylen, const std::string &after, t_Cursor &cursor, OutputFlag);
        unsigned int write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags);
        unsigned int write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags);
        void write(const std::string &str, const OutputFlag flags) const;
        void displayDiffOp(WINDOW *w, const int &y, const eLevenshteinOperator &op) const;

        void writeTopLine(const std::string &currentBuffer, short color) const;

        bool jumpToNextSearch(const JSonElement *current, bool &selectFound);
        bool jumpToNextSearch();
        unsigned int search(const SearchPattern &searchPattern);
        unsigned int search(const SearchPattern &searchPattern, const JSonElement *current);

        /**
         * get the screen size
        **/
        const t_Cursor getScreenSize() const;

        /**
         * Release ncurses
        **/
        void shutdown();

        void destroyAllSubWin();

        /**
         * get flags to be passed to write.
         * Contains indications on who to write item
        **/
        const OutputFlag getFlag(const JSonElement *item) const;
        const OutputFlag getFlag(const JSonElement *item, const JSonElement *selection) const;

    protected:
        inputResult selectUp();
        inputResult selectDown();
        inputResult selectPUp();
        inputResult selectPDown();
        inputResult expandSelection();
        inputResult collapseSelection();
        inputResult initSearch();
        inputResult nextResult();
        inputResult changeWindow(char, bool);

        void onResizeHandler();

        void setSelection(const JSonElement *);

        void computeDiff();
        std::deque<t_subWindow> subWindows;

        /**
         * currently searching pattern and its results
        **/
        const LevenshteinMatrice_base *diffMatrice;

        /**
         * Viewport start
        **/
        unsigned short nbInputs, selectedWin, workingWin;
        // TODO t_subWindow &workingSubwin, &selectedSubwin ??
};

