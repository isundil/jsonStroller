
#pragma once

#include <list>
#include <string>
#include <istream>

class AParams
{
    public:
        virtual bool isIgnoringUnicode() const =0;
        virtual bool isStrict() const =0;
};

class Params: public AParams
{
    public:
        Params(int ac, char **av);
        virtual ~Params();

        std::basic_istream<char> &getInput() const;
        bool isValid() const;
        bool isStrict() const;
        bool colorEnabled() const;

        static void usage(const std::string &) noexcept;

        const std::string &getProgName() const;
        bool isIgnoringUnicode() const;

    private:
        std::basic_istream<char> *input;
        const std::string progName;
        std::list<std::string> params;
        bool ignoreUnicode;
        bool colorMode;
        bool strict;
};

