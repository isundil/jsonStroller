#include <algorithm>
#include <sstream>
#include <regex>

#include "searchPattern.hh"
#include "jsonObjectEntry.hh"
#include "jsonPrimitive.hh"

SearchPattern::SearchPattern(const std::string &input): regex(nullptr), flags(0)
{
    size_t pos = 0;
    bool escaped = false;
    std::stringstream ss;

    for (pos =0; input[pos] && (input[pos] == ' ' || input[pos] == '\t'); ++pos); //trim leading characters
    for (; input[pos]; ++pos)
    {
        if (escaped)
        {
            if (input[pos] == '/')
                ss.put(input[pos]);
            else
                ss.put('\\').put(input[pos]);
            escaped = false;
        }
        else if (input[pos] == '\\')
            escaped = true;
        else if (input[pos] == '/')
        {
            pattern = ss.str();
            evalFlags(&input[pos +1]);
            return;
        }
        else
            ss.put(input[pos]);
    }
    pattern = ss.str();
}

SearchPattern::~SearchPattern()
{
    if (regex)
        delete regex;
}

void SearchPattern::evalFlags(const char *s)
{
    while (*s)
    {
        if (*s == 'i')
        {
            flags |= SearchPattern::FLAG_CASE;
            std::transform(pattern.begin(), pattern.end(), pattern.begin(), ::tolower);
        }
        else if (*s == 'w')
            flags |= SearchPattern::FLAG_WHOLEWORD;
        else if (*s == 'f')
            flags |= SearchPattern::FLAG_WHOLESTR;
        else if (*s == 'e')
            flags |= SearchPattern::FLAG_REGEX;
        else if (*s == 'b')
            typeFlag = SearchPattern::TYPE_BOOL;
        else if (*s == 'n')
            typeFlag = SearchPattern::TYPE_NUMBER;
        else if (*s == 's')
            typeFlag = SearchPattern::TYPE_STRING;
        else if (*s == 'o')
            typeFlag = SearchPattern::TYPE_OBJKEY;
        s++;
    }
    if (!regex && flags & SearchPattern::FLAG_REGEX)
    {
        std::regex_constants::syntax_option_type opts = std::regex_constants::ECMAScript;
        if (flags & FLAG_CASE)
            opts |= std::regex_constants::icase;
        regex = new std::regex(pattern, opts);
    }
}

bool SearchPattern::isEmpty() const
{ return pattern.empty(); }

bool SearchPattern::operator()(char a, char b)
{
    if (a == '\t')
        a = ' ';
    if (flags & SearchPattern::FLAG_CASE)
        return std::tolower(a) == b;
    return a == b;
}

bool SearchPattern::match(const std::string &str, const JSonElement *type) const
{
    if (typeFlag)
    {
        if (typeFlag == SearchPattern::TYPE_BOOL && !(dynamic_cast<const JSonPrimitive<bool> *>(type)))
            return false;
        else if (typeFlag == SearchPattern::TYPE_STRING && !(dynamic_cast<const JSonPrimitive<std::string> *>(type)))
            return false;
        else if (typeFlag == SearchPattern::TYPE_OBJKEY && !(dynamic_cast<const JSonObjectEntry *>(type)))
            return false;
        else if (typeFlag == SearchPattern::TYPE_NUMBER &&
                !(dynamic_cast<const JSonPrimitive<int> *>(type)) &&
                !(dynamic_cast<const JSonPrimitive<double> *>(type)) &&
                !(dynamic_cast<const JSonPrimitive<long long> *>(type)))
            return false;
    }
    if (flags & FLAG_REGEX)
        return regex_search(str, *regex);
    if ((flags & FLAG_WHOLESTR && str.length() != pattern.length())
            || pattern.length() > str.length())
        return false;
    if (flags & FLAG_WHOLEWORD && str.length() > pattern.length())
    {
        std::string pattern = " " +this->pattern +' ';
        return std::search(str.begin(), str.end(), pattern.begin(), pattern.end(), *this) != str.end();
    }
    return std::search(str.begin(), str.end(), pattern.begin(), pattern.end(), *this) != str.end();
}

const short SearchPattern::FLAG_CASE        = 1;
const short SearchPattern::FLAG_WHOLEWORD   = 2;
const short SearchPattern::FLAG_WHOLESTR    = 4;
const short SearchPattern::FLAG_REGEX       = 8;

const short SearchPattern::TYPE_BOOL        = 1;
const short SearchPattern::TYPE_NUMBER      = 2;
const short SearchPattern::TYPE_STRING      = 3;
const short SearchPattern::TYPE_OBJKEY      = 4;

