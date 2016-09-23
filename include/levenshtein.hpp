#pragma once

#include <string>
#include <list>
#include <limits.h>
#include "jsonElement.hh"
#include "levenshteinMatrice.hpp"

#define LEVENSHTEIN_SENSIBILITY (0.7f)

float levenshteinPercent(const std::string &a, const std::string &b);
template<class T> float levenshteinPercent(const std::list<T *> *a, const std::list<T *> *b);
bool levenshteinStrictCompare(const char &a, const char &b);
bool levenshteinStrictCompare(const JSonElement *a, const JSonElement *b);
bool levenshteinCompare(const char &a, const char &b);
bool levenshteinCompare(const JSonElement *a, const JSonElement *b);

template<typename SIZE, class ITERATOR, class SUBTYPE>
static LevenshteinMatrice<SIZE> *_levenshteinMatrice(const ITERATOR &aBegin, const ITERATOR &aEnd, const ITERATOR &bBegin, const ITERATOR &bEnd, const size_t lenA, const size_t lenB)
{
    size_t i, j;
    LevenshteinMatrice<SIZE> *matrice = new LevenshteinMatrice<SIZE>(lenA, lenB);
    ITERATOR a = aBegin;
    ITERATOR b;

    for (i =1; a != aEnd; ++i, ++a)
    {
        b = bBegin;
        for (j =1; b != bEnd; ++j, ++b)
            matrice->set(i, j, std::min(std::min(
                    matrice->get(i -1, j) +1,
                    matrice->get(i, j -1) +1),
                    matrice->get(i -1, j -1) + ((levenshteinCompare(*a, *b) > LEVENSHTEIN_SENSIBILITY) ? 0 : 1)));
    }
    return matrice;
};

template<typename SIZE, typename ITERATOR, class SUBTYPE>
static float _levenshteinPercent(ITERATOR aBegin, ITERATOR aEnd, ITERATOR bBegin, ITERATOR bEnd, size_t lenA, size_t lenB)
{
    const size_t maxSize = std::max(lenA, lenB);
    while (aBegin != aEnd && bBegin != bEnd && levenshteinCompare(*aBegin, *bBegin))
    {
        aBegin++;
        bBegin++;
        lenA--;
        lenB--;
    }
    if (!lenA && !lenB) return 1.f;
    if (!lenA) return (float) lenB / maxSize;
    if (!lenB) return (float) lenA / maxSize;
    LevenshteinMatrice<SIZE> *matrice = _levenshteinMatrice<SIZE, ITERATOR, SUBTYPE>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
    const SIZE result = matrice->result();

    delete matrice;
    return 1 - ((float)result / maxSize);
};

template<class T> float levenshteinPercent(const std::list<T *> *a, const std::list<T *> *b)
{
    const size_t lenA = a->size();
    const size_t lenB = b->size();
    typename std::list<T*>::const_iterator aBegin = a->cbegin();
    typename std::list<T*>::const_iterator aEnd = a->cend();
    typename std::list<T*>::const_iterator bBegin = b->cbegin();
    typename std::list<T*>::const_iterator bEnd = b->cend();

    if (lenA < UCHAR_MAX && lenB < UCHAR_MAX)
        return _levenshteinPercent<unsigned char, typename std::list<T *>::const_iterator, T *>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
    if (lenA < USHRT_MAX && lenB < USHRT_MAX)
        return _levenshteinPercent<unsigned short, typename std::list<T *>::const_iterator, T *>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
    return _levenshteinPercent<unsigned int, typename std::list<T *>::const_iterator, T *>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
}

template<class T>
LevenshteinMatrice_base *levenshteinShortestPath(const std::list<T*> *a, const std::list<T *> *b)
{
    const size_t lenA = a->size();
    const size_t lenB = b->size();

    if (lenA < UCHAR_MAX && lenB < UCHAR_MAX)
        return _levenshteinMatrice<unsigned char, typename std::list<T *>::const_iterator, T *>(a->cbegin(), a->cend(), b->cbegin(), b->cend(), lenA, lenB);
    if (lenA < USHRT_MAX && lenB < USHRT_MAX)
        return _levenshteinMatrice<unsigned short, typename std::list<T *>::const_iterator, T *>(a->cbegin(), a->cend(), b->cbegin(), b->cend(), lenA, lenB);
    return _levenshteinMatrice<unsigned int, typename std::list<T *>::const_iterator, T *>(a->cbegin(), a->cend(), b->cbegin(), b->cend(), lenA, lenB);
}

