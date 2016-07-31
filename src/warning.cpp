/**
 * warning.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "warning.hh"

Warning::Warning(const JsonException *w): what(w)
{ }

Warning::~Warning()
{ }

const JsonException &Warning::operator()() const
{ return *what; }

const JsonException *Warning::getPtrUnsafe() const
{ return what; }

