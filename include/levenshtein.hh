#pragma once

#include <string>
#include <list>
#include <limits.h>
#include "jsonElement.hh"

#include <iostream>

float levenshteinPercent(const std::string &a, const std::string &b);
template<class T> float levenshteinPercent(const std::list<T *> *a, const std::list<T *> *b);
bool levenshteinCompare(const char &a, const char &b);
bool levenshteinCompare(const JSonElement *a, const JSonElement *b);

template<typename SIZE, class T, class SUBTYPE>
static SIZE **_levenshteinMatrice(const T *a, const T *b, const size_t lenA, const size_t lenB)
{
    size_t i, j;
    SIZE **matrice = new SIZE*[lenA +1]();

    matrice[0] = new SIZE[lenB +1]();
    for (j=0; j <= lenB; j++)
        matrice[0][j] = j;
    i = 1;
    for (SUBTYPE it: *a)
    {
        j =1;
        matrice[i] = new SIZE[lenB +1]();
        matrice[i][0] = i;
        for (SUBTYPE jt: *b)
        {
            matrice[i][j] = std::min(std::min(
                    matrice[i -1][j] +1,
                    matrice[i][j -1] +1),
                    matrice[i -1][j -1] + ((levenshteinCompare(it, jt) > .9f) ? 0 : 1));
            j++;
        }
        i++;
    }

    for (size_t i=0; i <= lenA; ++i)
    {
        for (size_t j=0; j <= lenB; ++j)
            std::cerr << (size_t) matrice[i][j] << "\t";
        std::cerr << std::endl << "[";
    }
    return matrice;
};

template<typename SIZE, class T, class SUBTYPE>
static SIZE _levenshteinPercent(const T *a, const T *b, const size_t lenA, const size_t lenB)
{
    if (lenA == 0) return lenB;
    if (lenB == 0) return lenA;
    SIZE **matrice = _levenshteinMatrice<SIZE, T, SUBTYPE>(a, b, lenA, lenB);
    size_t i;
    const SIZE result = matrice[lenA][lenB];

    for (i=0; i < lenA; ++i)
        delete[] matrice[i];
    delete[] matrice;
    return 1 - (result / std::max(lenA, lenB));
};

template<class T> float levenshteinPercent(const std::list<T *> *a, const std::list<T *> *b)
{
    const size_t lenA = a->size();
    const size_t lenB = b->size();

    if (lenA < UCHAR_MAX && lenB < UCHAR_MAX)
        return _levenshteinPercent<unsigned char, std::list<T *>, T *>(a, b, lenA, lenB);
    if (lenA < USHRT_MAX && lenB < USHRT_MAX)
        return _levenshteinPercent<unsigned short, std::list<T *>, T *>(a, b, lenA, lenB);
    return _levenshteinPercent<unsigned int, std::list<T *>, T *>(a, b, lenA, lenB);
}

enum ePath: char
{
    add = '+',
    rem = '-',
    mod = '!',
    equ = '='
};

template<typename SIZE, class T, class SUBTYPE>
static std::list<ePath> _levenshteinShortestPath(const T *a, const T *b, const size_t lenA, const size_t lenB)
{
    std::list<ePath> result(std::max(lenA, lenB));

    if (lenA == 0 || lenB == 0)
    //TODO create deque<ePath>(std::max(lenA, lenB) populated with '-'
        ;
    SIZE **matrice = _levenshteinMatrice<SIZE, T, SUBTYPE>(a, b, lenA, lenB);
    size_t i;

    //TODO find shortest path


    // Clean matrice
    for (i=0; i < lenA; ++i)
        delete[] matrice[i];
    delete[] matrice;
    return result;
};

template<class T>
std::list<ePath> levenshteinShortestPath(const std::list<T*> *a, const std::list<T *> *b)
{
    const size_t lenA = a->size();
    const size_t lenB = b->size();

    if (lenA < UCHAR_MAX && lenB < UCHAR_MAX)
        return _levenshteinShortestPath<unsigned char, std::list<T *>, T *>(a, b, lenA, lenB);
    if (lenA < USHRT_MAX && lenB < USHRT_MAX)
        return _levenshteinShortestPath<unsigned short, std::list<T *>, T *>(a, b, lenA, lenB);
    return _levenshteinShortestPath<unsigned int, std::list<T *>, T *>(a, b, lenA, lenB);
}

