#include <curses.h>
#include "inputSequence.hh"

InputSequence::InputSequence()
{ }

InputSequence::InputSequence(const InputSequence &o): seq(o.seq)
{ }

InputSequence::~InputSequence()
{ }

InputSequence InputSequence::read()
{
    InputSequence result;
    const char *kname = nullptr;

    do
    {
        const int input = getch();

        if (input == -1)
            continue;

        kname = keyname(toupper(input));

        if (!result.seq.empty())
            result.seq += "-";
        result.seq += kname;
    }   while (!kname || *kname == '^');
    return result;
}

const std::string &InputSequence::key() const
{ return seq; }

