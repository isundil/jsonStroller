#include "jsonPrimitive.hh"

AJsonPrimitive::~AJsonPrimitive()
{}

template<> JSonPrimitive<double>::~JSonPrimitive() {}
template<> JSonPrimitive<bool>::~JSonPrimitive() {}
template<> JSonPrimitive<int>::~JSonPrimitive() {}
template<> JSonPrimitive<long long>::~JSonPrimitive() {}
template<> JSonPrimitive<std::string>::~JSonPrimitive() {}

template<> std::string JSonPrimitive<std::string>::stringify() const
{
    return value;
}

template<> std::string JSonPrimitive<double>::stringify() const
{
    return std::to_string(value);
}

template<> std::string JSonPrimitive<long long>::stringify() const
{
    return std::to_string(value);
}

template<> std::string JSonPrimitive<int>::stringify() const
{
    return std::to_string(value);
}

template<> std::string JSonPrimitive<bool>::stringify() const
{
    return value ? "true" : "false";
}

