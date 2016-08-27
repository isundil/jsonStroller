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
        void run(JSonElement *);

        bool jumpToNextSearch(const JSonElement *current, bool &selectFound);
        bool jumpToNextSearch();
        unsigned int search(const SearchPattern &search_pattern, const JSonElement *current);

        /**
         * Initialize ncurses
        **/
        void init();

        /**
         * Release ncurses
        **/
        void shutdown();

        /**
         * get flags to be passed to write.
         * Contains indications on who to write item
        **/
        const OutputFlag getFlag(const JSonElement *item) const;
        const OutputFlag getFlag(const JSonElement *item, const JSonElement *selection) const;

        unsigned int write(const int &x, const int &y, const char item, unsigned int maxWidth, OutputFlag flags);
        unsigned int write(const int &x, const int &y, const std::string &str, const size_t strlen, unsigned int maxWidth, const OutputFlag flags);
        void write(const std::string &str, const OutputFlag flags) const;
        bool writeKey(const std::string &key, const size_t keylen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, OutputFlag, unsigned int extraLen =0);
        bool writeKey(const std::string &key, const size_t keylen, const std::string &after, size_t afterlen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, OutputFlag flags);
        bool writeContainer(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *);
        bool writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, std::list<JSonElement *> * obj);
        bool redraw(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &, JSonElement *);

        Optional<bool> evalKey(int k);
        void checkSelection(const JSonElement *item, const std::pair<int, int> &cursor);

    protected:
        /**
         * Root item
        **/
        JSonElement *data;
        const JSonElement *selection;

        /**
         * Viewport start
        **/
        int scrollTop;

        /**
         * currently searching pattern and its results
        **/
        std::list<const JSonElement*> search_result;

        /**
         * prev/next items to be selected on up/down keys
        **/
        const JSonElement *select_up, *select_down;
};

