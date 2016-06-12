#pragma once

#include <string>
#include "jsonElement.hh"

template <typename T>
class JSonPrimitive: public JSonElement
{
    public:
        JSonPrimitive(T const &v);
        virtual ~JSonPrimitive();

    protected:
        const T value;
};

template<> class JSonPrimitive<float>: public JSonElement
{
    public:
        JSonPrimitive(const std::string &v);

    protected:
        const float value;
};

template<> class JSonPrimitive<long long int>: public JSonElement
{
    public:
        JSonPrimitive(const std::string &v);

    protected:
        const bool value;
};

template<> class JSonPrimitive<std::string>: public JSonElement
{
    public:
        JSonPrimitive(const std::string &v);

        virtual bool operator<(const JSonPrimitive<std::string> &other) const;

    protected:
        //TODO
        const std::string value;
};

template<> class JSonPrimitive<bool>: public JSonElement
{
    public:
        JSonPrimitive(bool v);

    protected:
        const bool value;
};

