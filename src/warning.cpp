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

Warning::Warning(const Warning &w): what(w.what), type(w.type), _filename(w._filename)
{ }

Warning::~Warning()
{ }

const JsonException &Warning::operator()() const
{ return what; }

const std::string &Warning::getType() const
{ return type; }

std::string Warning::getType(const std::exception &w)
{ return typeid(w).name(); }

std::string Warning::filename() const
{ return _filename; }

std::string Warning::filename(const std::string &f)
{ return _filename = f; }

