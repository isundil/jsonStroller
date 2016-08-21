#include <iostream>
#include "optional.hpp"

#define FAILED(got, op, expt) {std::cout << __FILE__ << ":" << __LINE__ << ": failed asserting " << got << " " << op << " expected " << expt << std::endl; return false; }

bool simpleTest()
{
    Optional<int> test = Optional<int>::of(42);

    if (test.absent())
        FAILED(test.absent(), "!=", true);
    if (test.get() != 42)
        FAILED(test.get(), "!=", 42);
    if (test.orValue(1) != 42)
        FAILED(test.get(), "!=", 42);
    test = Optional<int>::empty;
    if (!test.absent())
        FAILED(test.absent(), "!=", false);
    if (test.orValue(0) != 0)
        FAILED(test.get(), "!=", 0);
    return true;
}

int main()
{
    if (!simpleTest())
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}

