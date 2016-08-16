/**
 * outputFlag.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "outputFlag.hh"

const char OutputFlag::TYPE_UNKNOWN =0;
const char OutputFlag::TYPE_STRING  =1;
const char OutputFlag::TYPE_NUMBER  =2;
const char OutputFlag::TYPE_BOOL    =3;
const char OutputFlag::TYPE_OBJ     =4;
const char OutputFlag::TYPE_OBJKEY  =5;
const char OutputFlag::TYPE_ARR     =6;
const char OutputFlag::TYPE_NULL    =7;

const char OutputFlag::SPECIAL_NONE     =50;
const char OutputFlag::SPECIAL_SEARCH   =51;
const char OutputFlag::SPECIAL_ERROR    =52;

OutputFlag::OutputFlag(short m): mode(m)
{ }

OutputFlag::~OutputFlag()
{ }

bool OutputFlag::selected() const
{ return mode & OutputFlag::MODE_SELECTED; }

bool OutputFlag::selected(bool v)
{
    if (v)
        mode |= OutputFlag::MODE_SELECTED;
    else
        mode &= ~OutputFlag::MODE_SELECTED;
    return v;
}

bool OutputFlag::searched() const
{ return mode & OutputFlag::MODE_SEARCHED; }

bool OutputFlag::searched(bool v)
{
    if (v)
        mode |= OutputFlag::MODE_SEARCHED;
    else
        mode &= ~OutputFlag::MODE_SEARCHED;
    return v;
}

char OutputFlag::type() const
{ return _type; }

char OutputFlag::type(char t)
{ return _type = t; }

