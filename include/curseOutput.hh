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
template<class T> class Optional;

class CurseOutput
{
    public:
        CurseOutput(JSonElement *rootData, const Params &);
        virtual ~CurseOutput();

        /**
         * Display data, and shutdown ncurses at the end
        **/
        void run();
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
        bool redraw(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxWidth, const JSonElement *item);
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
        static unsigned int getNbLines(float nbChar, unsigned int maxWidth);
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
        unsigned int write(const int &x, const int &y, const JSonElement *item, unsigned int maxWidth, const OutputFlag);
        unsigned int write(const int &x, const int &y, const std::string &item, unsigned int maxWidth, const OutputFlag);
        unsigned int write(const int &x, const int &y, const char item, unsigned int maxWidth, const OutputFlag);
        bool writeKey(const std::string &key, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, OutputFlag, unsigned int extraLen =0);
        bool writeKey(const std::string &key, const std::string &after, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, OutputFlag);
        bool writeContainer(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *);
        bool writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const std::list<JSonElement *> * obj);

        /**
         * find next search occurence and select it (wich cause viewport to move, eventually)
         * Have it own redraw because may have to write message
        **/
        bool jumpToNextSearch(bool scanParent, bool redraw, const JSonElement *initial_selection);
        /**
         * prompt for user input, and return it
         * @throws noInputException if user use a kill-key (to exit buffer)
        **/
        const std::string search();
        /**
         * Write a message on the last line, using color
        **/
        void writeBottomLine(const std::string &currentBuffer, short color) const;

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
        const JSonElement *data;
        /**
         * selected item
        **/
        const JSonElement *selection;
        /**
         * currently searching pattern
        **/
        std::string search_pattern;
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

