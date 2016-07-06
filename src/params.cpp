#include <fstream>
#include <iostream>
#include <sstream>
#include "params.hh"

Params::Params(int ac, char **av) :progName(*av), params(std::list<std::string>(ac -1))
{
    bool written = false;
    std::stringstream *input = nullptr;

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

bool Params::isValid() const
{
    return true;
}

