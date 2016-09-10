#include "levenshtein.hh"

float levenshteinPercent(const std::string &a, const std::string &b)
{
    const size_t lenA = a.size();
    const size_t lenB = b.size();

    if (lenA < UCHAR_MAX && lenB < UCHAR_MAX)
        return _levenshteinPercent<unsigned char, std::string, char>(&a, &b, lenA, lenB);
    if (lenA < USHRT_MAX && lenB < USHRT_MAX)
        return _levenshteinPercent<unsigned short, std::string, char>(&a, &b, lenA, lenB);
    return _levenshteinPercent<unsigned int, std::string, char>(&a, &b, lenA, lenB);
}

bool levenshteinCompare(const char &a, const char &b)
{
    return a == b;
}

bool levenshteinCompare(const JSonElement *a, const JSonElement *b)
{
    return a->diff(b) > .7f;
}

