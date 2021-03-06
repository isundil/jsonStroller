/**
 * jsonPrimitive.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include "jsonElement.hh"

class Null {}; //Null primitive

class AJSonPrimitive: public JSonElement
{
    public:
        AJSonPrimitive(JSonElement *parent);
        virtual ~AJSonPrimitive();

        virtual std::string getTypeStr() const =0;
        bool sameType(const AJSonPrimitive *other) const;
};

template <typename T>
class JSonPrimitive: public AJSonPrimitive
{
    public:
        JSonPrimitive(JSonContainer *parent, T const &v);
        virtual ~JSonPrimitive();

        /**
         * get value as raw type
        **/
        T getValue() const;
        /**
         * get stringified value
        **/
        std::string stringify() const;

        std::string getTypeStr() const;

        bool operator<(const JSonPrimitive<T> &other) const;
        bool operator==(const JSonPrimitive<T> &other) const;

    protected:
        /**
         * convert raw type to string
        **/
        virtual std::string toString() const;

        /**
         * raw value
        **/
        const T value;
        /**
         * stringified value
        **/
        const std::string stringValue;
};

template<typename T>
JSonPrimitive<T>::JSonPrimitive(JSonContainer *parent, T const &v): AJSonPrimitive(parent), value(v), stringValue(toString())
{ }

template<typename T>
T JSonPrimitive<T>::getValue() const
{ return value; }

template<typename T>
std::string JSonPrimitive<T>::stringify() const
{ return stringValue; }

template<typename T>
bool JSonPrimitive<T>::operator<(const JSonPrimitive<T> &other) const
{ return value < other.value; }

template<typename T>
bool JSonPrimitive<T>::operator==(const JSonPrimitive<T> &other) const
{ return value == other.value; }

