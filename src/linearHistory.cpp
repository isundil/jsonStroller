/**
 * linearHistory.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "linearHistory.hh"

LinearHistory::LinearHistory(): willReset(false), line(1)
{ }

LinearHistory::~LinearHistory()
{ }

unsigned int LinearHistory::currentLine() const
{ return line; }

void LinearHistory::put(char item)
{
    if (willReset)
    {
        reset();
        line++;
        willReset = false;
    }
    if (item == '\n')
        willReset = true;
    else
        WrappedBuffer<char, ERROR_HISTORY_LEN>::put(item);
}

void LinearHistory::put(char items[], unsigned int count)
{
    if (willReset)
    {
        reset();
        line++;
        willReset = false;
    }
    for (unsigned int i=0; i < count; ++i)
    {
        if (items[i] == '\n')
        {
            if (i < count -1)
            {
                willReset = true;
                put(&items[i +1], count -i);
                return;
            }
            else
            {
                willReset = true;
                return;
            }
        }
    }
    WrappedBuffer<char, ERROR_HISTORY_LEN>::put(items, count);
}

