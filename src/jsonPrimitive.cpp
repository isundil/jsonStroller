/**
 * jsonPrimitive.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "jsonPrimitive.hh"

AJSonPrimitive::AJSonPrimitive(JSonElement *p): JSonElement(p)
{}

AJSonPrimitive::~AJSonPrimitive()
{}

bool AJSonPrimitive::sameType(const AJSonPrimitive *o) const
{ return getTypeStr() == o->getTypeStr(); }

template<> JSonPrimitive<Null>::~JSonPrimitive() {}
template<> JSonPrimitive<double>::~JSonPrimitive() {}
template<> JSonPrimitive<bool>::~JSonPrimitive() {}
template<> JSonPrimitive<int>::~JSonPrimitive() {}
template<> JSonPrimitive<long long>::~JSonPrimitive() {}
template<> JSonPrimitive<std::string>::~JSonPrimitive() {}

template<> std::string JSonPrimitive<std::string>::toString() const
{ return value; }

template<> std::string JSonPrimitive<Null>::toString() const
{ return "null"; }

template<> std::string JSonPrimitive<double>::toString() const
{
    return std::to_string(value);
}

template<> std::string JSonPrimitive<long long>::toString() const
{
    return std::to_string(value);
}

template<> std::string JSonPrimitive<int>::toString() const
{
    return std::to_string(value);
}

template<> std::string JSonPrimitive<bool>::toString() const
{ return value ? "true" : "false"; }

template<> std::string JSonPrimitive<std::string>::getTypeStr() const
{ return "str"; }

template<> std::string JSonPrimitive<Null>::getTypeStr() const
{ return "null"; }

template<> std::string JSonPrimitive<double>::getTypeStr() const
{ return "number"; }

template<> std::string JSonPrimitive<long long>::getTypeStr() const
{ return "number"; }

template<> std::string JSonPrimitive<int>::getTypeStr() const
{ return "number"; }

template<> std::string JSonPrimitive<bool>::getTypeStr() const
{ return "bool"; }

