#pragma once

#include <string>

unsigned int levenshtein(const char *a, const size_t lenA, const char *b, const size_t lenB);
unsigned int levenshtein(const std::string &a, const std::string &b);
float levenshteinPercent(const std::string &strA, const std::string &strB);

