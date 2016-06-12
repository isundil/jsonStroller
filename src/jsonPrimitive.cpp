#include "jsonPrimitive.hh"

template<> JSonPrimitive<float>::~JSonPrimitive() {}
template<> JSonPrimitive<bool>::~JSonPrimitive() {}
template<> JSonPrimitive<int>::~JSonPrimitive() {}

template<> JSonPrimitive<long long>::~JSonPrimitive() {}
template<> JSonPrimitive<std::string>::~JSonPrimitive() {}

