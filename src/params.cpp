
#include <iostream>
#include <sstream>
#include "params.hpp"

Params::Params(int ac, char **av) :progName(*av), params(std::list<std::string>(ac -1))
{
    bool written = false;
    std::stringstream *input = nullptr;

    while (*(++av))
    {
        std::string tmp(*av);
        if (!input)
        {
            params.push_back(tmp);
            if (tmp == "--")
            {
                input = new std::stringstream();
            }
        }
        else
        {
            if (written)
                input->write(" ", sizeof(char));
            else
                written = true;
            input->write(tmp.c_str(), sizeof(char) * tmp.size());
        }
    }
    this->input = input;
}

std::basic_istream<char> &Params::getInput() const
{
    return input == nullptr ? std::cin : *input;
}

