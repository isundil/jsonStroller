/**
 * params.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <list>
#include <string>
#include <istream>

class AParams
{
    public:
        /**
         * true if --ascii
        **/
        virtual bool isIgnoringUnicode() const =0;
        /**
         * false if -W
        **/
        virtual bool isStrict() const =0;
        /**
         * true if --color match conditions
        **/
        virtual bool colorEnabled() const =0;
};

class Params: public AParams
{
    public:
        Params(char **av);
        virtual ~Params();

        /**
         * Interpret input
        **/
        virtual bool read();

        /**
         * retun input
         * can be file stream (-f), stringstream ( -- INPUT), or std::cin (none)
        **/
        std::list<std::basic_istream<char>*> getInputs();

        /**
         * false if invalid argument is passed
        **/
        bool isValid() const;

        /**
         * print usage
         * @param program name
        **/
        virtual void usage() const noexcept;

        /**
         * print version number
         * @param program name
        **/
        virtual void version() const noexcept;

        /**
         * get argv[0]
        **/
        const std::string &getProgName() const;

        /**
         * flags
        **/
        bool isStrict() const;
        bool colorEnabled() const;
        bool isIgnoringUnicode() const;

    private:
        /**
         * input stream
         * can be null for stdin
        **/
        std::list<std::basic_istream<char>*> inputs;

        const std::string progName;
        std::list<std::string> params;

        bool ignoreUnicode;
        bool colorMode;
        bool strict;
};

