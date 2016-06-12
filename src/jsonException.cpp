#include <string>
#include "jsonException.hh"

JsonException::JsonException(unsigned long long pos): offset(pos)
{ }

JsonFormatException::JsonFormatException(char character, unsigned long long pos): JsonException(pos), c(character)
{ }

JsonEscapedException::JsonEscapedException(char character, unsigned long long pos): JsonFormatException(character, pos)
{ }

const char *JsonFormatException::what() const noexcept
{
    std::string res = "Error: unexpected escaped char '";
    res += c +"' at offset " +offset;
    return res.c_str();
}

