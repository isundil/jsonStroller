#pragma once

#include <deque>
#include "curseOutput.hh"
#include "levenshtein.hpp"

class LevenshteinMatrice_base;

typedef struct
{
    std::string fileName;
    JSonElement *root;
    const JSonElement *selection, *select_up, *select_down;
    WINDOW *innerWin, *outerWin;
    std::list<const JSonElement*> searchResults;
    int scrollTop;
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

        void checkSelection(const JSonElement *item, const std::pair<int, int> &cursor);

        void loop();

        bool redraw();
        bool redraw(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &, JSonElement *);
        bool redrawCurrent(const std::pair<unsigned int, unsigned int> &screenSize);

        bool writeContainer(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &, const JSonContainer *);
        bool writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, std::list<JSonElement*> *_item);
        bool writeKey(const std::string &key, const size_t keylen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags, unsigned int extraLen =0);
        bool writeKey(const std::string &key, const size_t keylen, const std::string &after, size_t afterlen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags);
        unsigned int write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags);
        unsigned int write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags);
        void write(const std::string &str, const OutputFlag flags) const;

        void writeTopLine(const std::string &currentBuffer, short color) const;

        bool jumpToNextSearch(const JSonElement *current, bool &selectFound);
        bool jumpToNextSearch();
        unsigned int search(const SearchPattern &searchPattern);
        unsigned int search(const SearchPattern &searchPattern, const JSonElement *current);

        /**
         * get the screen size
        **/
        const std::pair<unsigned int, unsigned int> getScreenSize() const;

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

