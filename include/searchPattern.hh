#pragma once

#include <string>
#include <regex>

class JSonElement;

class SearchPattern
{
    public:
        SearchPattern(const std::string &);
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
        std::regex *regex;
        short flags;
        short typeFlag;

        static const short FLAG_CASE;
        static const short FLAG_WHOLEWORD;
        static const short FLAG_WHOLESTR;
        static const short FLAG_REGEX;

        static const short TYPE_BOOL;
        static const short TYPE_NUMBER;
        static const short TYPE_STRING;
        static const short TYPE_OBJKEY;
};

