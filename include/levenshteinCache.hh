#pragma once

#include <map>
#include "levenshtein.hpp"

enum eLevenshteinOperator: char
{
    add = '+',
    rem = '-',
    mod = '!',
    equ = '='
};

template<class T> class LevenshteinCache
{
    public:
        ~LevenshteinCache()
        {}

        void push(const T key, const eLevenshteinOperator &value)
        {
            cache[key] = value;
        }

    public:
        static LevenshteinCache<T> *instance()
        {
            if (LevenshteinCache<JSonElement *>::_instance)
                return LevenshteinCache<JSonElement *>::_instance;
            return LevenshteinCache<JSonElement *>::_instance = new LevenshteinCache<JSonElement *>();
        }

    private:
        LevenshteinCache()
        { }

    private:
        std::map<const T, eLevenshteinOperator> cache;

    private:
        static LevenshteinCache<T> *_instance;
};

