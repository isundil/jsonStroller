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
    WrappedBuffer<char, ERROR_HISTORY_LEN>::put(items, count);
}

