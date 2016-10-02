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
#include "optional.hpp"
#include "outputFlag.hh"
#include "params.hh"
#include "inputSequence.hh"

class JSonElement;
class JSonContainer;
class JSonArray;
class JSonObject;
class SearchPattern;

enum inputResult: char
{
    redraw      =0
    ,redrawAll  =1
    ,quit       =2
    ,nextInput  =3
};

typedef std::pair<unsigned int, unsigned int> t_Cursor;

class CurseOutput
{
    public:
        CurseOutput(const Params &);
        virtual ~CurseOutput();

        /**
         * Called on SIG* while displaying data
         * Do not use (private).
        **/
        bool onsig(int signo);

    protected:
        /**
         * until kill-input, display data and read user inputs
        **/
        virtual void loop();

        /**
         * Initialize ncurses
        **/
        virtual void init();

        /**
         * Release ncurses
        **/
        virtual void shutdown() =0;

        /**
         * return false if bottom of screen is touched
         * redraw all data
        **/
        virtual bool redraw() =0;
        /**
         * Like redraw, but append a message on the last line
        **/
        bool redraw(const std::string &errorMsg);
        /**
         * redraw item and children
        **
        virtual bool redraw(t_Cursor &, const t_Cursor &maxWidth, JSonElement *item) =0;
        */

        /**
         * Wait for input
         * @return false if ncurses should stop
        **/
        inputResult readInput();
        inputResult evalKey(const InputSequence &k);

        virtual inputResult selectUp() =0;
        virtual inputResult selectDown() =0;
        virtual inputResult selectPUp() =0;
        virtual inputResult selectPDown() =0;
        virtual inputResult expandSelection() =0;
        virtual inputResult collapseSelection() =0;
        virtual inputResult initSearch() =0;
        virtual inputResult nextResult() =0;
        virtual inputResult changeWindow(char direction, bool cycle) =0;

        /**
         * get the screen size
        **/
        virtual const t_Cursor getScreenSize() const;
        const t_Cursor getScreenSizeUnsafe() const;

        /**
         * set the select_up and select_down pointers, scroll to selection if it is above view port
        **
        virtual void checkSelection(const JSonElement *item, const t_Cursor &) =0;
        */

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
        virtual const OutputFlag getFlag(const JSonElement *item) const =0;

        /**
         * Write data
        **/
        /**
         * Warning: this one does not check line height, because he's not aware of cursor position
        **/
        virtual void write(const std::string &str, const OutputFlag flags) const =0;
        unsigned int write(const int &x, const int &y, JSonElement *item, unsigned int maxWidth, const OutputFlag);
        virtual unsigned int write(const int &x, const int &y, const std::string &item, const size_t len, unsigned int maxWidth, const OutputFlag) =0;
        virtual unsigned int write(const int &x, const int &y, const char item, unsigned int maxWidth, const OutputFlag) =0;
        /*
        virtual bool writeKey(const std::string &key, const size_t keylen, t_Cursor &cursor, const t_Cursor &maxWidth, OutputFlag, unsigned int extraLen =0) =0;
        */
        virtual bool writeKey(const std::string &key, const size_t keylen, const std::string &after, const size_t afterlen, t_Cursor &cursor, const t_Cursor &maxWidth, OutputFlag) =0;
        virtual bool writeKey(const std::string &key, const size_t keylen, const std::string &after, t_Cursor &cursor, const t_Cursor &maxWidth, OutputFlag);

        /*
        virtual bool writeContainer(t_Cursor &, const t_Cursor &maxSize, const JSonContainer *) =0;
        virtual bool writeContent(t_Cursor &cursor, const t_Cursor &maxSize, std::list<JSonElement *> * obj) =0;
        */

        /**
         * prompt for user input, and return it
         * @throws noInputException if user use a kill-key (to exit buffer)
        **/
        const SearchPattern *inputSearch();

        /**
         * find occurences of search result and fill this#search_result
        **/
        virtual unsigned int search(const SearchPattern &, const JSonElement *) =0;

        /**
         * Write a message on the last line, using color
        **/
        virtual void writeTopLine(const std::string &currentBuffer, short color) const;
        virtual void writeBottomLine(const std::string &currentBuffer, short color) const;
        virtual void writeBottomLine(const std::wstring &currentBuffer, short color) const;

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
         * initialized colors
        **/
        std::set<char /* OutputFlag::TYPE_SOMETHING */> colors;

        /**
         * Selection helpers
         * Used for moving viewport
        **/
        bool selectFound, selectIsLast;

        class SelectionOutOfRange { };
};

void _resizeFnc(int signo);

