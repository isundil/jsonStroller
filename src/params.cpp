/**
 * params.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include <fstream>
#include <iostream>
#include <sstream>
#include <curses.h>
#include <unistd.h>
#include "params.hh"

#include "config.h"

Params::Params(char **av): progName(*av), strict(true), diffMode(false)
{
    av++;
    while (*av)
    {
        params.push_back(*av);
        av++;
    }
}

Params::~Params()
{
    for (std::pair<std::string, std::basic_istream<char>*> in : inputs)
        delete in.second;
}

bool Params::read()
{
    bool written = false;
    std::stringstream *input = nullptr;
    ignoreUnicode = false;
    colorMode = sorted = true;

    for (std::list<std::string>::const_iterator i = params.cbegin(); i != params.cend(); i++)
    {
        std::string tmp(*i);
        if (!input)
        {
            if (tmp == "--")
                inputs["ARGS"] = input = new std::stringstream();
            else if (tmp == "-W")
                strict = false;
            else if (tmp == "--ascii")
                ignoreUnicode = true;
            else if (tmp == "--color")
                colorMode = true;
            else if (tmp == "--diff")
                diffMode = true;
            else if (tmp == "--help" || tmp == "-h")
            {
                usage();
                return false;
            }
            else if (tmp == "--version" || tmp == "-v")
            {
                version();
                return false;
            }
            else if (tmp == "--keep-order")
                sorted = false;
            else if (tmp.find("--color=") == 0)
            {
                std::string mode = tmp.substr(8);
                if (mode == "always")
                    colorMode = true;
                else if (mode == "never")
                    colorMode = false;
                else
                    throw std::runtime_error("Invalid option for --color: " +mode);
            }
            else if (tmp.find("-") == 0)
                throw std::runtime_error("Invalid argument: " +tmp);
            else
            {
                std::ifstream *in = new std::ifstream(tmp);
                if (!in->is_open())
                {
                    delete in;
                    throw std::runtime_error("Cannot open " +tmp +" for reading");
                }
                if (inputs.contains(tmp))
                {
                    delete in;
                    throw std::runtime_error("Cannot compare " +tmp +" and " +tmp +" files (path are identical)");
                }
                else
                    inputs[tmp] = in;
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
    if (diffMode)
    {
        if (inputs.size() > 3)
        {
            std::cerr << progName << ": more than 3 files to compare" << std::endl;
            usage();
            return false;
        }
        if (inputs.size() < 2)
        {
            std::cerr << progName << ": not enough files to compare" << std::endl;
            usage();
            return false;
        }
    }
    return true;
}

IndexedDeque Params::getInputs() const
{
    if (!inputs.empty())
        return inputs;
    IndexedDeque result;
    result["STDIN"] = &std::cin;
    return result;
}

void Params::usage() const noexcept
{
    std::cout << "Usage: "
    << progName << " [OPTIONS]" << std::endl
    << "or: " << progName << " [OPTIONS] FILENAME" << std::endl
    << "or: " << progName << " [OPTIONS] -- INPUT" << std::endl
    << "or: " << progName << " --diff [OPTIONS] FILENAME FILENAME [FILENAME]" << std::endl
    << "Read json input and print it using ncurse" << std::endl << std::endl
    << "if not INPUT nor FILENAME, use standard input" << std::endl << std::endl

    << "  FILENAME\t\tread input from filename instead of stdin" << std::endl
    << "  INPUT\t\t\tuse this as input instead of stdin" << std::endl
    << "  -W \t\t\tconsider continuing on non-blocking errors" << std::endl
    << "  --ascii\t\tignore unicode values" << std::endl
    << "  --keep-order\t\tdo not sort objects by key" << std::endl
    << "  --color[=MODE]\tcolorize output, MODE can be never or always (default when ommited)" << std::endl
    << "  -v, -version\t\tdisplay version information" << std::endl
    << "  -h, --helph\t\tshow this message and exit" << std::endl << std::endl

    << "Examples:" << std::endl
    << STROLL_PROGNAME << " f.json\tOutput f.json's content" << std::endl << std::endl
    << STROLL_PROGNAME << " --diff fileA.json fileB.json\tcompare fileA.json and fileB.json" << std::endl << std::endl

    << "Report bugs to <isundill@gmail.com>" << std::endl;
}

void Params::version() const noexcept
{
    std::cout << STROLL_PROGNAME << " (jsonstroller suite) " << VERSION << " generated on " << VERSIONDATE << std::endl << std::endl

    << "Copyright (C) 2016 Free Software Foundation, Inc." << std::endl
    << "License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>." << std::endl

    << "This is free software: you are free to change and redistribute it." << std::endl
    << "There is NO WARRANTY, to the extent permitted by law." << std::endl << std::endl

    << "Written by isundil <isundill@gmail.com>." << std::endl;
}

bool Params::isStrict() const
{ return strict; }

bool Params::colorEnabled() const
{ return colorMode; }

bool Params::sortObjects() const
{ return sorted; }

bool Params::isIgnoringUnicode() const
{ return ignoreUnicode; }

const std::string &Params::getProgName() const
{ return progName; }

bool Params::isDiff() const
{ return diffMode; }

