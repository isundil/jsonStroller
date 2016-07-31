/**
 * linearHistory.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include "config.h"
#include "wrappedBuffer.hpp"

class LinearHistory: public WrappedBuffer<char, ERROR_HISTORY_LEN>
{
    public:
        LinearHistory();
        ~LinearHistory();

        /**
         * Get line number
        **/
        unsigned int currentLine() const;

        /**
         * Insert char(s)
         * If new-line found, reinitialize and increment line
        **/
        void put(char item);
        void put(char item[], unsigned int count);

    private:
        /**
         * True if new line found, and should reset at new put
        **/
        bool willReset;
        /**
         * Current line
        **/
        unsigned int line;
};

