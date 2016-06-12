#pragma once

#include <string>
#include "jsonElement.hh"

template <typename T>
class JSonPrimitive: public JSonElement
{
    public:
        JSonPrimitive(T const &v);
        virtual ~JSonPrimitive();

        T getValue() const;

        bool operator<(const JSonPrimitive<T> &other) const;
        bool operator==(const JSonPrimitive<T> &other) const;

    protected:
        const T value;
};

template<typename T>
JSonPrimitive<T>::JSonPrimitive(T const &v): value(v)
{ }

template<typename T>
T JSonPrimitive<T>::getValue() const
{
    return value;
}

template<typename T>
bool JSonPrimitive<T>::operator<(const JSonPrimitive<T> &other) const
{
    return value < other.value;
}

template<typename T>
bool JSonPrimitive<T>::operator==(const JSonPrimitive<T> &other) const
{
    return value == other.value;
}

