#include "levenshtein.hh"

unsigned int levenshtein(const char *a, const size_t lenA, const char *b, const size_t lenB)
{
    int **matrice = new int*[lenA]();
    matrice[0] = new int[lenB]();
    for (size_t j=0; j < lenB; j++)
        matrice[0][j] = j;
    for (size_t i=1; i < lenA; ++i)
    {
        matrice[i] = new int[lenB]();
        matrice[i][0] = i;
        for (size_t j=1; j < lenB; ++j)
            matrice[i][j] = std::min(std::min(
                    matrice[i -1][j] +1,
                    matrice[i][j -1] +1),
                    matrice[i -1][j -1] + (a[i] == b[j] ? 0 : 1));
    }
    return matrice[lenA -1][lenB -1];
}

unsigned int levenshtein(const std::string &strA, const std::string &strB)
{
    return levenshtein(strA.c_str(), strA.size(), strB.c_str(), strB.size());
}

float levenshteinPercent(const std::string &strA, const std::string &strB)
{
    return 1 - (levenshtein(strA, strB) / std::max(strA.size(), strB.size()));
}

