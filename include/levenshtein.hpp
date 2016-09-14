#pragma once

#include <string>
#include <list>
#include <limits.h>
#include "jsonElement.hh"

#define LEVENSHTEIN_SENSIBILITY (0.7f)

float levenshteinPercent(const std::string &a, const std::string &b);
template<class T> float levenshteinPercent(const std::list<T *> *a, const std::list<T *> *b);
bool levenshteinStrictCompare(const char &a, const char &b);
bool levenshteinStrictCompare(const JSonElement *a, const JSonElement *b);
bool levenshteinCompare(const char &a, const char &b);
bool levenshteinCompare(const JSonElement *a, const JSonElement *b);

template<typename SIZE, class ITERATOR, class SUBTYPE>
static SIZE **_levenshteinMatrice(const ITERATOR &aBegin, const ITERATOR &aEnd, const ITERATOR &bBegin, const ITERATOR &bEnd, const size_t lenA, const size_t lenB)
{
    size_t i, j;
    SIZE **matrice = new SIZE*[lenA +1]();
    ITERATOR a = aBegin;
    ITERATOR b;

    matrice[0] = new SIZE[lenB +1]();
    for (j=0; j <= lenB; j++)
        matrice[0][j] = j;
    for (i =1; a != aEnd; ++i, ++a)
    {
        matrice[i] = new SIZE[lenB +1]();
        matrice[i][0] = i;
        b = bBegin;
        for (j =1; b != bEnd; ++j, ++b)
            matrice[i][j] = std::min(std::min(
                    matrice[i -1][j] +1,
                    matrice[i][j -1] +1),
                    matrice[i -1][j -1] + ((levenshteinCompare(*a, *b) > LEVENSHTEIN_SENSIBILITY) ? 0 : 1));
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
    SIZE **matrice = _levenshteinMatrice<SIZE, ITERATOR, SUBTYPE>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
    size_t i;
    const SIZE result = matrice[lenA][lenB];

    for (i=0; i < lenA; ++i)
        delete[] matrice[i];
    delete[] matrice;
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

enum ePath: char
{
    add = '+',
    rem = '-',
    mod = '!',
    equ = '='
};

template<typename SIZE, class ITERATOR, class SUBTYPE>
static size_t _levenshteinShortestPath(std::list<ePath> &result, ITERATOR aBegin, ITERATOR aEnd, ITERATOR bBegin, ITERATOR bEnd, size_t lenA, size_t lenB)
{
    const size_t initLenA = lenA;
    const size_t initLenB = lenB;
    result.clear();

    while (aBegin != aEnd && bBegin != bEnd && levenshteinCompare(*aBegin, *bBegin))
    {
        aBegin++;
        bBegin++;
        lenA--;
        lenB--;
    }
    if (!lenA && !lenB)
    {
        for (size_t i=0; i < initLenA; ++i)
            result.push_back(ePath::equ);
        return 0;
    }
    else if (!lenA)
    {
        size_t i;
        for (i=0; i < initLenB - lenB; ++i)
            result.push_back(ePath::equ);
        for (; i < initLenB; ++i)
            result.push_back(ePath::rem);
        return lenB;
    }
    else if (!lenB)
    {
        size_t i;
        for (i=0; i < initLenA - lenA; ++i)
            result.push_back(ePath::equ);
        for (; i < initLenA; ++i)
            result.push_back(ePath::add);
        return lenA;
    }
    SIZE **matrice = _levenshteinMatrice<SIZE, ITERATOR, SUBTYPE>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
    size_t i = lenA;
    size_t j = lenB;
    const size_t levenDist = matrice[i][j];

    while (i || j)
    {
        if (i && (!j || matrice[i][j] > matrice[i-1][j]))
        {
            result.push_front(ePath::add);
            --i;
        }
        else if (j && (!i || matrice[i][j] > matrice[i][j -1]))
        {
            result.push_front(ePath::rem);
            --j;
        }
        else if (i && j)
        {
            result.push_front(matrice[i][j] == matrice[i-1][j-1] ? ePath::equ : ePath::mod);
            --i;
            --j;
        }
        else if (i)
        {
            result.push_front(ePath::add);
            --i;
        }
        else if (j)
        {
            result.push_front(ePath::rem);
            --j;
        }
    }

    for (i = initLenA - lenA; i; --i)
        result.push_front(ePath::equ);

    // Clean matrice
    for (i=0; i < lenA +1; ++i)
        delete[] matrice[i];
    delete[] matrice;
    return levenDist;
};

template<class T>
size_t levenshteinShortestPath(std::list<ePath> &result, const std::list<T*> *a, const std::list<T *> *b)
{
    const size_t lenA = a->size();
    const size_t lenB = b->size();

    if (lenA < UCHAR_MAX && lenB < UCHAR_MAX)
        return _levenshteinShortestPath<unsigned char, typename std::list<T *>::const_iterator, T *>(result, a->cbegin(), a->cend(), b->cbegin(), b->cend(), lenA, lenB);
    if (lenA < USHRT_MAX && lenB < USHRT_MAX)
        return _levenshteinShortestPath<unsigned short, typename std::list<T *>::const_iterator, T *>(result, a->cbegin(), a->cend(), b->cbegin(), b->cend(), lenA, lenB);
    return _levenshteinShortestPath<unsigned int, typename std::list<T *>::const_iterator, T *>(result, a->cbegin(), a->cend(), b->cbegin(), b->cend(), lenA, lenB);
}

