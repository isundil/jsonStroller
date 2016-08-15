#pragma once

#include <string>

class JSonElement;

class SearchPattern
{
    public:
        SearchPattern(const char *);
        ~SearchPattern();

        bool isEmpty() const;
        bool match(const std::string &other, const JSonElement *) const;

        /**
         * Comparison function, for std::search use
        **/
        bool operator()(char a, char b);

    private:
        void evalFlags(const char *);

        std::string pattern;
        short flags;
        short typeFlag;

        static const short FLAG_CASE;
        static const short TYPE_BOOL;
        static const short TYPE_NUMBER;
        static const short TYPE_STRING;
        static const short TYPE_OBJKEY;
};

