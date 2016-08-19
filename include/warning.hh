/**
 * warning.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include "jsonException.hh"

class Warning
{
    public:
        Warning(const Warning &);
        Warning(const JsonException &);
        ~Warning();

        /**
         * get exception representing non-blocking error
        **/
        const JsonException &operator()() const;
        /**
         * Because of copy, we can't get warning typeinfo
        **/
        const std::string &getType() const;

        /**
         * return type of given exception
        **/
        static std::string getType(const std::exception &);

        std::string filename() const;
        std::string filename(const std::string &filename);

    private:
        JsonException what;
        std::string type;
        std::string _filename;
};

