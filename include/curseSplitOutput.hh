#pragma once

#include <deque>
#include "curseOutput.hh"

class CurseSplitOutput: public CurseOutput
{
    public:
        CurseSplitOutput(const Params &);
        virtual ~CurseSplitOutput();

        /**
         * Display data, and shutdown ncurses at the end
        **/
        void run(const std::deque<std::string> &, const std::deque<JSonElement *> &);

        bool redraw();

        /**
         * get the screen size
        **/
        const std::pair<unsigned int, unsigned int> getScreenSize() const;

        /**
         * Initialize ncurses
        **/
        void init();

        /**
         * Release ncurses
        **/
        void shutdown();

        void destroyAllSubWin();

    protected:
        std::deque<JSonElement *> roots;
        std::deque<JSonElement *> selections;
        std::deque<std::string> fileNames;
        std::deque<WINDOW *> subwindows;
        size_t nbInputs, currentWin;
};

