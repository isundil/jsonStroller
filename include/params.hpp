
#pragma once

#include <list>
#include <string>
#include <istream>

class Params
{
    public:
        Params(int ac, char **av);

        std::basic_istream<char> &getInput() const;

    private:
        std::basic_istream<char> *input;
        const std::string progName;
        std::list<std::string> params;
};

