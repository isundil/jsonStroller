#pragma once

#include <exception>
#include "wrappedBuffer.hpp"

class EofException: public std::exception
{ };

class JsonException: public std::exception
{
    public:
        JsonException(const std::string &what, unsigned long long offset, WrappedBuffer<char> &buf);

        std::string getHistory() const;
        const char *what() const noexcept;

    protected:
        const unsigned long long offset;
        const WrappedBuffer<char> history;
        const std::string _what;
};

class JsonNotJsonException: public JsonException
{
    public:
        JsonNotJsonException(unsigned long long offet, WrappedBuffer<char> &h);
};

class JsonUnexpectedException: public JsonException
{
    public:
        JsonUnexpectedException(const char expected, unsigned long long offset, WrappedBuffer<char> &h);
};

class JsonFormatException: public JsonException
{
    public:
        JsonFormatException(unsigned long long offset, WrappedBuffer<char> &h);
};

class JsonEscapedException: public JsonException
{
    public:
        JsonEscapedException(char c, unsigned long long offset, WrappedBuffer<char> &h);

    protected:
        const char c;
};

