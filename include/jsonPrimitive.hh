#pragma once

#include "jsonElement.hh"

class AJSonPrimitive
{
    public:
        virtual ~AJSonPrimitive();
};

template <typename T>
class JSonPrimitive: public JSonElement, public AJSonPrimitive
{
    public:
        JSonPrimitive(JSonContainer *parent, T const &v);
        virtual ~JSonPrimitive();

        T getValue() const;
        virtual std::string stringify() const;

        bool operator<(const JSonPrimitive<T> &other) const;
        bool operator==(const JSonPrimitive<T> &other) const;

    protected:
        const T value;
};

template<typename T>
JSonPrimitive<T>::JSonPrimitive(JSonContainer *parent, T const &v): JSonElement(parent), value(v)
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

