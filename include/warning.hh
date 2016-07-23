#pragma once

#include "jsonException.hh"

class Warning
{
    public:
        Warning(const JsonException *);
        ~Warning();

        const JsonException &operator()() const;
        const JsonException *getPtrUnsafe() const;

    private:
        const JsonException *what;
};

