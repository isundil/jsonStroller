#include <stdexcept>
#include "unicode.hpp"
#include "jsonException.hh"

unsigned char hexbyte(const char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    throw std::invalid_argument(JsonHexvalueException::msg(c));
}

