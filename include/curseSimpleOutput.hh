/**
 * curseOutput.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include "curseOutput.hh"

class CurseSimpleOutput: public CurseOutput
{
    public:
        CurseSimpleOutput(const Params &);
        ~CurseSimpleOutput();

        bool redraw();

        /**
         * Display data, and shutdown ncurses at the end
        **/
        void run(JSonElement *, const std::string &inputName);

        bool jumpToNextSearch(const JSonElement *current, bool &selectFound);
        bool jumpToNextSearch();
        unsigned int search(const SearchPattern &search_pattern, const JSonElement *current);

        /**
         * Release ncurses
        **/
        void shutdown();

        const t_Cursor getScreenSize() const;
        /**
         * get flags to be passed to write.
         * Contains indications on who to write item
        **/
        const OutputFlag getFlag(const JSonElement *item) const;
        const OutputFlag getFlag(const JSonElement *item, const JSonElement *selection) const;

        unsigned int write(const unsigned int &x, const unsigned int &y, const char item, unsigned int maxWidth, OutputFlag flags);
        unsigned int write(const unsigned int &x, const unsigned int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags);
        void write(const std::string &str, const OutputFlag flags) const;
        bool writeKey(const std::string &key, const size_t keylen, t_Cursor &cursor, OutputFlag, unsigned int extraLen =0);
        bool writeKey(const std::string &key, const size_t keylen, const std::string &after, size_t afterlen, t_Cursor &cursor, OutputFlag flags);
        bool writeKey(const std::string &key, const size_t keylen, const std::string &after, t_Cursor &cursor, OutputFlag);
        bool writeContainer(t_Cursor &, const JSonContainer *);
        bool writeContent(t_Cursor &cursor, std::list<JSonElement *> * obj);
        bool redraw(t_Cursor &, JSonElement *);
        void checkSelection(const JSonElement *item, const t_Cursor &cursor);

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

        /**
         * Root item
        **/
        JSonElement *data;
        const JSonElement *selection;

        /**
         * Viewport start
        **/
        unsigned int scrollTop;

        /**
         * currently searching pattern and its results
        **/
        std::list<const JSonElement*> search_result;

        /**
         * prev/next items to be selected on up/down keys
        **/
        const JSonElement *select_up, *select_down;

        /**
         * input name
        **/
        std::string inputName;

        /**
         * Selection helpers
         * Used for moving viewport
        **/
        bool selectFound, selectIsLast;
};

