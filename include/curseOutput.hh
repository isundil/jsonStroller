/**
 * curseOutput.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <ncurses.h>
#include <fstream>
#include <list>
#include <set>
#include <map>
#include "outputFlag.hh"
#include "params.hh"

class JSonElement;
class JSonContainer;
class JSonArray;
class JSonObject;
class SearchPattern;

class CurseOutput
{
    public:
        CurseOutput(const Params &);
        virtual ~CurseOutput();

        /**
         * Display data, and shutdown ncurses at the end
        **/
        void run(JSonElement *);
        void run(std::list<JSonElement *>);

        /**
         * Called on SIG* while displaying data
         * Do not use (private).
        **/
        bool onsig(int signo);

    private:
        /**
         * until kill-input, display data and read user inputs
        **/
        virtual void loop();

    protected:
        /**
         * Initialize ncurses
        **/
        void init();

        /**
         * Release ncurses
        **/
        void shutdown();

        /**
         * return false if bottom of screen is touched
         * redraw all data
        **/
        bool redraw();
        /**
         * Like redraw, but append a message on the last line
        **/
        bool redraw(const std::string &errorMsg);
        /**
         * redraw item and children
        **/
        bool redraw(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxWidth, JSonElement *item);

        /**
         * Wait for input
         * @return false if ncurses should stop
        **/
        bool readInput();

        /**
         * get the screen size
        **/
        const std::pair<unsigned int, unsigned int> getScreenSize() const;

        /**
         * set the select_up and select_down pointers, scroll to selection if it is above view port
        **/
        void checkSelection(const JSonElement *item, const std::pair<int, int>&);

        /**
         * Return the number of lines written while writting nbChar bytes
         * @param nbChar col written
         * @param @maxWidth screen width
         * @return the number of line written
        **/
        static unsigned int getNbLines(const size_t nbChar, unsigned int maxWidth);

        /**
         * get flags to be passed to write.
         * Contains indications on who to write item
        **/
        const OutputFlag getFlag(const JSonElement *item) const;

        /**
         * Write data
        **/
        /**
         * Warning: this one does not check line height, because he's not aware of cursor position
        **/
        void write(const std::string &str, const OutputFlag flags) const;
        unsigned int write(const int &x, const int &y, JSonElement *item, unsigned int maxWidth, const OutputFlag);
        unsigned int write(const int &x, const int &y, const std::string &item, const size_t len, unsigned int maxWidth, const OutputFlag);
        unsigned int write(const int &x, const int &y, const char item, unsigned int maxWidth, const OutputFlag);
        bool writeKey(const std::string &key, const size_t keylen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, OutputFlag, unsigned int extraLen =0);
        bool writeKey(const std::string &key, const size_t keylen, const std::string &after, const size_t afterlen, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, OutputFlag);
        bool writeKey(const std::string &key, const size_t keylen, const std::string &after, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, OutputFlag);
        bool writeContainer(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *);
        bool writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, std::list<JSonElement *> * obj);

        /**
         * find next search occurence and select it (wich cause viewport to move, eventually)
         * Have it own redraw because may have to write message
        **/
        bool jumpToNextSearch(const JSonElement *initial_selection, bool &);
        bool jumpToNextSearch();

        /**
         * prompt for user input, and return it
         * @throws noInputException if user use a kill-key (to exit buffer)
        **/
        const SearchPattern *inputSearch();

        /**
         * find occurences of search result and fill this#search_result
        **/
        unsigned int search(const SearchPattern &, const JSonElement *);

        /**
         * Write a message on the last line, using color
        **/
        void writeBottomLine(const std::string &currentBuffer, short color) const;
        void writeBottomLine(const std::wstring &currentBuffer, short color) const;

        /**
         * unfold all item's parents
        **/
        void unfold(const JSonElement *);


        // Fields
        /**
         * collapsed items
        **/
        std::set<const JSonContainer *> collapsed;

        /**
         * Root item
        **/
        JSonElement *data;

        /**
         * selected item
        **/
        const JSonElement *selection;

        /**
         * currently searching pattern and its results
        **/
        std::list<const JSonElement*> search_result;

        /**
         * prev/next items to be selected on up/down keys
        **/
        const JSonElement *select_up, *select_down;

        /**
         * Program params (ac/av)
        **/
        const Params &params;

        /**
         * ncurses stuff
        **/
        SCREEN *screen;
        FILE *screen_fd;

        /**
         * Used by signals to stop reading input and shutdown ncurses
        **/
        bool breakLoop;

        /**
         * Viewport start
        **/
        int scrollTop;

        /**
         * initialized colors
        **/
        std::set<char /* OutputFlag::TYPE_SOMETHING */> colors;

        /**
         * Selection helpers
         * Used for moving viewport
        **/
        bool selectFound, selectIsLast;
};

