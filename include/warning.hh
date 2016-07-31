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

    private:
        JsonException what;
        std::string type;
};

