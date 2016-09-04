#include <curses.h>
#include "inputSequence.hh"

InputSequence::InputSequence()
{ }

InputSequence::InputSequence(const InputSequence &o): _key(o._key)
{ }

InputSequence::~InputSequence()
{ }

InputSequence InputSequence::read()
{
    InputSequence result;

    result._key = getch();
    return result;
}

int InputSequence::key() const
{ return _key; }

