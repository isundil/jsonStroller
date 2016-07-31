/**
 * jsonException.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <exception>
#include "config.h"
#include "linearHistory.hh"

class EofException: public std::exception
{ };

class JsonException: public std::exception
{
    public:
        JsonException(const std::string &what, unsigned long long offset, LinearHistory &buf);

        std::string getHistory() const;
        const char *what() const noexcept;

        unsigned int currentLine() const;

    protected:
        const unsigned long long offset;
        const LinearHistory history;
        const std::string _what;
};

class JsonHexvalueException: public JsonException
{
    public:
        JsonHexvalueException(const std::string &what, unsigned long long offset, LinearHistory &hist);
        JsonHexvalueException(const char what, unsigned long long offset, LinearHistory &hist);

        static std::string msg(char c);
};

class JsonNotJsonException: public JsonException
{
    public:
        JsonNotJsonException(unsigned long long offet, LinearHistory &h);
};

class JsonUnexpectedException: public JsonException
{
    public:
        JsonUnexpectedException(const char expected, unsigned long long offset, LinearHistory &h);
};

class JsonFormatException: public JsonException
{
    public:
        JsonFormatException(unsigned long long offset, LinearHistory &h);
};

class JsonEscapedException: public JsonException
{
    public:
        JsonEscapedException(char c, unsigned long long offset, LinearHistory &h);

    protected:
        const char c;
};

