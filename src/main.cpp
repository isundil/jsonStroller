
#include <iostream>
#include "params.hpp"

int main(int ac, char **av)
{
    Params *params = new Params(ac, av);

    std::basic_istream<char> &ss = params->getInput();
    while (ss.good())
    {
        std::cout << "[" << (char) ss.get() << "]";
    }
    std::cout << std::endl;
}

