#pragma once

#include <exception>

class EofException: std::exception
{ };

class JsonException: std::exception
{
    public:
        JsonException(unsigned long long offset);

    protected:
        const unsigned long long offset;
};

class JsonFormatException: JsonException
{
    public:
        JsonFormatException(char character, unsigned long long offset);
        const char *what() const noexcept;

    protected:
        const char c;
};

class JsonEscapedException: JsonFormatException
{
    public:
        JsonEscapedException(char character, unsigned long long offset);
};

