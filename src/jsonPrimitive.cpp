#include "jsonPrimitive.hh"

JSonPrimitive<std::string>::JSonPrimitive(std::string const &v): value(v)
{ }

bool JSonPrimitive<std::string>::operator<(const JSonPrimitive<std::string> &other) const
{
    return value < other.value;
}

JSonPrimitive<bool>::JSonPrimitive(bool v): value(v)
{ }

JSonPrimitive<float>::JSonPrimitive(const std::string &v): value(std::stof(v))
{ }

JSonPrimitive<long long int>::JSonPrimitive(const std::string &v): value(std::stol(v))
{ }

