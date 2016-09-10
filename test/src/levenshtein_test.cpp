#include <iostream>
#include "levenshtein.hh"

#define FAILED(got, op, expt) {std::cout << __FILE__ << ":" << __LINE__ << ": failed asserting " << got << " " << op << " expected " << expt << std::endl; return false; }

bool doTest()
{
    float pc;
    unsigned int lev;

    /*
    if ((lev = levenshtein("coucou", "coucou")) != 0)
        FAILED(lev, "!=", 0);
    if ((pc = levenshteinPercent("coucou", "coucou")) != 1)
        FAILED(pc, "!=", 1);
    if ((lev = levenshtein("coocou", "coucou")) != 1)
        FAILED(lev, "!=", 1);
    if ((lev = levenshtein("abcdefghijk", "zabcdefghijk")) != 1)
        FAILED(lev, "!=", 1);
    if ((lev = levenshtein("zabcdefghijk", "abcdefghijk")) != 1)
        FAILED(lev, "!=", 1);
    if ((lev = levenshtein("zabcdefghijk", "zabcdkfghijk")) != 1)
        FAILED(lev, "!=", 1);
    if ((lev = levenshtein("a", "zabcdkfghijk")) != 11)
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


