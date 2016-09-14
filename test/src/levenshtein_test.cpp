#include <iostream>
#include "levenshtein.hpp"

#define FAILED(got, op, expt) {std::cout << __FILE__ << ":" << __LINE__ << ": failed asserting " << got << " " << op << " expected " << expt << std::endl; return false; }

/*
size_t levenshteinShortestPath(const std::string &a, const std::string &b)
{
    std::list<ePath> r;
    return levenshteinShortestPath(r, a, b);
}
*/

bool doTest()
{
    /*
    float pc;
    unsigned int lev;

    if ((lev = levenshteinShortestPath("coucou", "coucou")) != 0)
        FAILED(lev, "!=", 0);
    if ((pc = levenshteinShortestPath("coucou", "coucou")) != 1)
        FAILED(pc, "!=", 1);
    if ((lev = levenshteinShortestPath("coocou", "coucou")) != 1)
        FAILED(lev, "!=", 1);
    if ((lev = levenshteinShortestPath("abcdefghijk", "zabcdefghijk")) != 1)
        FAILED(lev, "!=", 1);
    if ((lev = levenshteinShortestPath("zabcdefghijk", "abcdefghijk")) != 1)
        FAILED(lev, "!=", 1);
    if ((lev = levenshteinShortestPath("zabcdefghijk", "zabcdkfghijk")) != 1)
        FAILED(lev, "!=", 1);
    if ((lev = levenshteinShortestPath("a", "zabcdkfghijk")) != 11)
        FAILED(lev, "!=", 11);
    */
    return true;
}

int main()
{
    if (!doTest())
        exit(EXIT_FAILURE);
    exit(EXIT_SUCCESS);
}


