/**
 * jsonException.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <string>
#include "jsonException.hh"

JsonException::JsonException(const std::string &wh, unsigned long long pos, LinearHistory &h): offset(pos), history(h), _what(wh)
{ }

JsonHexvalueException::JsonHexvalueException(const std::string &what, unsigned long long o, LinearHistory &h): JsonFormatException(o, h)
{
    _what = what;
}

JsonHexvalueException::JsonHexvalueException(char c, unsigned long long o, LinearHistory &h): JsonFormatException(o, h)
{
    _what = JsonHexvalueException::msg(c);
}

JsonFormatException::JsonFormatException(unsigned long long pos, LinearHistory &h):
    JsonException("invalid value", pos, h)
{ }

JsonNotJsonException::JsonNotJsonException(unsigned long long pos, LinearHistory &h):
    JsonException("expected json entry, got token", pos, h)
{ }

JsonEscapedException::JsonEscapedException(char ch, unsigned long long pos, LinearHistory &h):
    JsonException("unexpected escaped char " +c, pos, h), c(ch)
{ }

JsonUnexpectedException::JsonUnexpectedException(const char expected, unsigned long long offset, LinearHistory &h): JsonException("expected " +expected, offset, h)
{ }

std::string JsonHexvalueException::msg(char c)
{
    std::string what = "invalid hex value '";
    what += c + '\'';
    return what;
}

unsigned int JsonException::currentLine() const
{ return history.currentLine(); }

unsigned int JsonException::currentCol() const
{ return history.totalSize(); }

std::string JsonException::getHistory() const
{ return history.toString(); }

const char *JsonException::what() const noexcept
{ return _what.c_str(); }

