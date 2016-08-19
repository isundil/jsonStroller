#pragma once

#include "curseOutput.hh"

class CurseSplitOutput: public CurseOutput
{
    public:
        CurseSplitOutput(const Params &);
        virtual ~CurseSplitOutput();

        /**
         * Display data, and shutdown ncurses at the end
        **/
        void run(std::list<JSonElement *>);

        bool redraw();
};

