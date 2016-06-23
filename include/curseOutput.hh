#pragma once

#include <fstream>
#include <ios>
#include <ncurses.h>

class JSonElement;
template<class T> class Optional;

class CurseOutput
{
    public:
        CurseOutput(JSonElement *rootData);
        virtual ~CurseOutput();

        void run();
        bool onsig(int signo);

    private:
        typedef Optional<std::pair<Optional<const std::string>, const JSonElement *> > t_nextKey;
        virtual void loop();

    protected:
        void init();
        void shutdown();
        void redraw();
        /**
         * return false if bottom of screen is touched
        **/
        bool redraw(std::pair<int, int> &, const std::pair<int, int>&, const JSonElement *);
        bool readInput();
        void getScreenSize(std::pair<int, int> &, std::pair<int, int> &);
        static CurseOutput::t_nextKey findNext(const JSonElement *);
        void write(const int &x, const int &y, const JSonElement *item, bool selected);
        void write(const int &x, const int &y, const std::string &item, bool selected);
        void writeKey(const std::string &key, std::pair<int, int> &cursor, bool selected);

        const JSonElement *data, *selection;
        SCREEN *screen;
        FILE *screen_fd;
        bool breakLoop;
        std::pair<std::pair<unsigned int, unsigned int>, const JSonElement *> topleft;
        const unsigned int indentLevel;

        //FIXME optimize
        const JSonElement *select_up, *select_down;
        bool selectFound;
};

