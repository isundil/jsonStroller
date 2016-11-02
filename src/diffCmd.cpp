#include <string.h>
#include "diffCmd.hh"

DiffCmd::DiffCmd(): allocatedArgv(nullptr)
{ }

DiffCmd::~DiffCmd()
{
    char **av = allocatedArgv;

    if (av)
    {
        while (*av)
        {
            free(*av);
            av++;
        }
        delete[] allocatedArgv;
    }
}

void DiffCmd::add(const std::string &str)
{ argv.push_back(str); }

std::string DiffCmd::getFile() const
{ return argv[0]; }

std::deque<std::string> DiffCmd::getArgv() const
{ return argv; }

char **DiffCmd::computeArgv(const std::deque<std::string> &inputFiles)
{
    if (allocatedArgv)
        return allocatedArgv;
    unsigned int nbParams = 0,
                 currentParam = 0;

    for (std::string &i : argv)
    {
        if (i == "{}")
            nbParams += inputFiles.size();
        else
            nbParams++;
    }
    allocatedArgv = new char*[nbParams +1];
    for (std::string &i : argv)
    {
        if (i == "{}")
            for(std::string input: inputFiles)
                allocatedArgv[currentParam++] = strdup(input.c_str());
        else
            allocatedArgv[currentParam++] = strdup(i.c_str());
    }
    allocatedArgv[currentParam] = nullptr;
    return allocatedArgv;
}

unsigned long long DiffCmd::hashString(const std::string &s)
{
    unsigned long long hash =0;

    for (char c: s)
        hash = (hash << 5) + c;

    return hash;
}

