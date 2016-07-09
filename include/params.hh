
#pragma once

#include <list>
#include <string>
#include <istream>

class Params
{
    public:
        Params(int ac, char **av);
        virtual ~Params();

        std::basic_istream<char> &getInput() const;
        bool isValid() const;

        static void usage(const std::string &) noexcept;

        const std::string &getProgName() const;

    private:
        std::basic_istream<char> *input;
        const std::string progName;
        std::list<std::string> params;
};

