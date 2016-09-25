#pragma once

#include <string>

#ifndef  LEVENSHTEIN_SENSIBILITY
# define LEVENSHTEIN_SENSIBILITY (0.7f)
#endif //LEVENSHTEIN_SENSIBILITY

size_t levenshtein(const std::string &a, const std::string &b);
float levenshteinPercent(const std::string &a, const std::string &b);

