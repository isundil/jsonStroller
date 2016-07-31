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

        unsigned int currentLine() const;

        void put(char item);
        void put(char item[], unsigned int count);

    private:
        bool willReset;
        unsigned int line;
};

