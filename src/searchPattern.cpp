#include <algorithm>
#include <sstream>
#include "searchPattern.hh"

#include "jsonObjectEntry.hh"
#include "jsonPrimitive.hh"

SearchPattern::SearchPattern(const wchar_t *input): flags(0)
{
    size_t pos = 0;
    bool escaped = false;
    std::wstringstream ss;

    for (pos =0; input[pos]; ++pos)
    {
        if (escaped)
        {
            if (input[pos] == L'/')
                ss.put(input[pos]);
            else
                ss.put(L'\\').put(input[pos]);
            escaped = false;
        }
        else if (input[pos] == L'\\')
            escaped = true;
        else if (input[pos] == L'/')
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
{ }

void SearchPattern::evalFlags(const wchar_t *s)
{
    while (*s)
    {
        if (*s == L'i')
        {
            flags |= SearchPattern::FLAG_CASE;
            std::transform(pattern.begin(), pattern.end(), pattern.begin(), ::tolower);
        }
        else if (*s == L'b')
            typeFlag = SearchPattern::TYPE_BOOL;
        else if (*s == L'n')
            typeFlag = SearchPattern::TYPE_NUMBER;
        else if (*s == L's')
            typeFlag = SearchPattern::TYPE_STRING;
        else if (*s == L'o')
            typeFlag = SearchPattern::TYPE_OBJKEY;
        else if (*s == L'w')
            flags |= SearchPattern::FLAG_WHOLEWORD;
        else if (*s == L'f')
            flags |= SearchPattern::FLAG_WHOLESTR;
        s++;
    }
}

bool SearchPattern::isEmpty() const
{ return pattern.empty(); }

bool SearchPattern::operator()(wchar_t a, wchar_t b)
{
    if (a == L'\t')
        a = L' ';
    if (flags & SearchPattern::FLAG_CASE)
        return std::tolower(a) == b;
    return a == b;
}

bool SearchPattern::match(const std::string &str, const JSonElement *type) const
{
    // Init str
    const size_t size = str.size();
    wchar_t tmpBuf[size];
    mbstowcs(&tmpBuf[0], str.c_str(), size);
    std::wstring wstr = std::wstring(tmpBuf, size);

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
    if ((flags & FLAG_WHOLESTR && wstr.length() != pattern.length())
            || pattern.length() > wstr.length())
        return false;
    if (flags & FLAG_WHOLEWORD && wstr.length() > pattern.length())
    {
        std::wstring pattern = L' ' +this->pattern +L' ';
        return std::search(wstr.begin(), wstr.end(), pattern.begin(), pattern.end(), *this) != wstr.end();
    }
    return std::search(wstr.begin(), wstr.end(), pattern.begin(), pattern.end(), *this) != wstr.end();
}

const short SearchPattern::FLAG_CASE        = 1;
const short SearchPattern::FLAG_WHOLEWORD   = 2;
const short SearchPattern::FLAG_WHOLESTR    = 4;

const short SearchPattern::TYPE_BOOL        = 1;
const short SearchPattern::TYPE_NUMBER      = 2;
const short SearchPattern::TYPE_STRING      = 3;
const short SearchPattern::TYPE_OBJKEY      = 4;

