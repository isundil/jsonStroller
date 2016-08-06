/**
 * jsonException.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <exception>
#include "config.h"
#include "linearHistory.hh"

/**
 * Reach end of input prematurelly
**/
class EofException: public std::exception
{ };

/**
 * std json parse exception
**/
class JsonException: public std::exception
{
    public:
        JsonException(const std::string &what, unsigned long long offset, LinearHistory &buf);

        std::string getHistory() const;
        const char *what() const noexcept;

        unsigned int currentLine() const;
        unsigned int currentCol() const;

    protected:
        const unsigned long long offset;
        const LinearHistory history;
        std::string _what;
};

/**
 * Expected JSon value (primitive / object / array), got crap
**/
class JsonNotJsonException: public JsonException
{
    public:
        JsonNotJsonException(unsigned long long offet, LinearHistory &h);
};

/**
 * Expected char, got crap
**/
class JsonUnexpectedException: public JsonException
{
    public:
        JsonUnexpectedException(const char expected, unsigned long long offset, LinearHistory &h);
};

/**
 * cannot interpret input as boolean/number
**/
class JsonFormatException: public JsonException
{
    public:
        JsonFormatException(unsigned long long offset, LinearHistory &h);
};

/**
 * Invalid hexadecimal value
**/
class JsonHexvalueException: public JsonFormatException
{
    public:
        JsonHexvalueException(const std::string &what, unsigned long long offset, LinearHistory &hist);
        JsonHexvalueException(const char what, unsigned long long offset, LinearHistory &hist);

        static std::string msg(char c);
};

/**
 * unknown escaped entry in string
 * (eg. \crap)
**/
class JsonEscapedException: public JsonException
{
    public:
        JsonEscapedException(char c, unsigned long long offset, LinearHistory &h);

    protected:
        const char c;
};

