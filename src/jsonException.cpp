#include <string>
#include "jsonException.hh"

JsonException::JsonException(const std::string &wh, unsigned long long pos, WrappedBuffer<char> &h): offset(pos), history(h), _what(wh)
{ }

JsonFormatException::JsonFormatException(unsigned long long pos, WrappedBuffer<char> &h):
    JsonException("invalid value", pos, h)
{ }

JsonNotJsonException::JsonNotJsonException(unsigned long long pos, WrappedBuffer<char> &h):
    JsonException("expected json entry, got token", pos, h)
{ }

JsonEscapedException::JsonEscapedException(char ch, unsigned long long pos, WrappedBuffer<char> &h):
    JsonException("unexpected escaped char " +c, pos, h), c(ch)
{ }

JsonUnexpectedException::JsonUnexpectedException(const char expected, unsigned long long offset, WrappedBuffer<char> &h): JsonException("expected " +expected, offset, h)
{ }

std::string JsonException::getHistory() const
{
    return history.toString();
}

const char *JsonException::what() const noexcept
{
    return _what.c_str();
}

