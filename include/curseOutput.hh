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

        void run();
        bool onsig(int signo);

    private:
        virtual void loop();

    protected:
        void init();
        void shutdown();
        /**
         * return false if bottom of screen is touched
        **/
        bool redraw();
        bool redraw(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxWidth, const JSonElement *, const JSonContainer *);
        bool readInput();
        void getScreenSize(std::pair<unsigned int, unsigned int> &) const;
        void getScreenSize(std::pair<unsigned int, unsigned int> &, std::pair<int, int> &) const;
        void checkSelection(const JSonElement *item, const JSonElement *parent, const std::pair<int, int>&);
        static unsigned int getNbLines(float nbChar, unsigned int maxWidth);
        const OutputFlag getFlag(const JSonElement *item) const;
        void write(const std::string &str, unsigned int maxWidth, const OutputFlag flags) const;
        unsigned int write(const int &x, const int &y, const JSonElement *item, unsigned int maxWidth, const OutputFlag);
        unsigned int write(const int &x, const int &y, const std::string &item, unsigned int maxWidth, const OutputFlag);
        unsigned int write(const int &x, const int &y, const char item, unsigned int maxWidth, const OutputFlag);
        bool writeKey(const std::string &key, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, OutputFlag, unsigned int extraLen =0);
        bool writeKey(const std::string &key, const std::string &after, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, OutputFlag);
        bool writeContainer(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *);
        bool writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const std::list<JSonElement *> * obj);

        void jumpToNextSearch();
        const std::string search();
        void initSearch(const std::string &currentBuffer) const;

        std::set<const JSonContainer *> collapsed;

        const JSonElement *data, *selection;
        const Params &params;
        std::string search_pattern;
        SCREEN *screen;
        FILE *screen_fd;
        bool breakLoop;
        int topleft;
        std::set<char> colors;

        const JSonElement *select_up, *select_down;
        bool selectFound, selectIsLast, selectIsFirst;
};

