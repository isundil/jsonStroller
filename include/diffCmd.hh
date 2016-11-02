#pragma once

#include <string>
#include <deque>

class DiffCmd
{
    public:
        DiffCmd();
        ~DiffCmd();

        /**
         * Append parameter to cmd params
        **/
        void add(const std::string &param);

        /**
         * file to pass to execvp to construct executable path from PATH or ./
        **/
        std::string getFile() const;
        /**
         * get args to pass to execvp
        **/
        std::deque<std::string> getArgv() const;

        /**
         * lazy-compute char*argv[] from File &a and File &b
        **/
        char **computeArgv(const std::deque<std::string> &inputFiles);

        /**
         * Hash string
        **/
        static unsigned long long hashString(const std::string &s);

    private:
        std::deque<std::string> argv;
        char **allocatedArgv;
};

