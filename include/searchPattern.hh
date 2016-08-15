#pragma once

#include <string>

class JSonElement;

class SearchPattern
{
    public:
        SearchPattern(const wchar_t *);
        ~SearchPattern();

        bool isEmpty() const;
        bool match(const std::string &other, const JSonElement *) const;

        /**
         * Comparison function, for std::search use
        **/
        bool operator()(wchar_t a, wchar_t b);

    private:
        void evalFlags(const wchar_t *);

        std::wstring pattern;
        short flags;
        short typeFlag;

        static const short FLAG_CASE;
        static const short FLAG_WHOLEWORD;
        static const short FLAG_WHOLESTR;

        static const short TYPE_BOOL;
        static const short TYPE_NUMBER;
        static const short TYPE_STRING;
        static const short TYPE_OBJKEY;
};

