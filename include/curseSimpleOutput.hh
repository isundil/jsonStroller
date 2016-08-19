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

        /**
         * Root item
        **/
        JSonElement *data;

        /**
         * Initialize ncurses
        **/
        void init();

        /**
         * Release ncurses
        **/
        void shutdown();
};

