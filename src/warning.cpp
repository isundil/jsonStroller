/**
 * warning.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <typeinfo>
#include "warning.hh"

Warning::Warning(const JsonException &w): what(w)
{
    type = Warning::getType(w);
}

Warning::~Warning()
{ }

const JsonException &Warning::operator()() const
{ return what; }

const std::string &Warning::getType() const
{ return type; }

std::string Warning::getType(const std::exception &w)
{ return typeid(w).name(); }

