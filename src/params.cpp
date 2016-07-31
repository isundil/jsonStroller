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

Params::Params(int ac, char **av) :progName(*av), params(std::list<std::string>(ac -1)), strict(true)
{
    bool written = false;
    std::stringstream *input = nullptr;
    ignoreUnicode = false;
    colorMode = isatty(1);

    while (*(++av))
    {
        std::string tmp(*av);
        if (!input)
        {
            if (tmp == "-f")
            {
                tmp = *(++av);
                if (!*av)
                    throw std::runtime_error("Invalid use of -f without argument");
                std::ifstream *in = new std::ifstream(tmp);
                if (!in->is_open())
                {
                    delete in;
                    throw std::runtime_error("Cannot open " +tmp +" for reading");
                }
                this->input = in;
            }
            else if (tmp == "--")
            {
                input = new std::stringstream();
            }
            else if (tmp == "-W")
                strict = false;
            else if (tmp == "--ascii")
                ignoreUnicode = true;
            else if (tmp == "--color")
                colorMode = true;
            else if (tmp.find("--color=") == 0)
            {
                std::string mode = (*av) + 8;
                if (mode == "always")
                    colorMode = true;
                else if (mode == "never")
                    colorMode = false;
                else if (mode == "auto")
                    colorMode = isatty(1);
                else
                    throw std::runtime_error("Invalid option for --color: " +mode);
            }
            else
                throw std::runtime_error("Invalid argument: " +tmp);
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
    if (!this->input)
        this->input = input;
}

Params::~Params()
{
    if (input)
        delete input;
}

std::basic_istream<char> &Params::getInput() const
{
    if (input != nullptr)
        return *input;
    return std::cin;
}

void Params::usage(const std::string &progName) noexcept
{
    std::cout << "Usage: " << progName << " [OPTIONS] [--] INPUT" << std::endl;
    std::cout << "\t\t-f filename\tread input from (filename) instead of stdin" << std::endl;
    std::cout << "\t\t--ascii\tignore unicode values" << std::endl;
    std::cout << "\t\t--color[=MODE]\tColorize output, MODE can be always (default when ommited), never or auto (default if --color is ommited)" << std::endl;
    std::cout << "\t\t-W\tdisable strict mode (warning does not interrupt reading)" << std::endl;
}

bool Params::isValid() const
{ return true; }

bool Params::isStrict() const
{ return strict; }

bool Params::colorEnabled() const
{ return colorMode; }

bool Params::isIgnoringUnicode() const
{ return ignoreUnicode; }

const std::string &Params::getProgName() const
{ return progName; }

