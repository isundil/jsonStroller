#include <climits>
#include "levenshteinMatrice.hpp"
#include "jsonObjectEntry.hh"

size_t levenshtein(const std::string &a, const std::string &b)
{
    int **matrice = new int*[a.size() +1]();
    matrice[0] = new int[b.size() +1]();
    for (size_t j=0; j <= b.size(); j++)
        matrice[0][j] = j;
    for (size_t i=1; i <= a.size(); ++i)
    {
        matrice[i] = new int[b.size() +1]();
        matrice[i][0] = i;
        for (size_t j=1; j <= b.size(); ++j)
            matrice[i][j] = std::min(std::min(
                    matrice[i -1][j] +1,
                    matrice[i][j -1] +1),
                    matrice[i -1][j -1] + (a[i -1] == b[j -1] ? 0 : 1));
    }

    const size_t result = matrice[a.size()][b.size()];
    for (size_t i=0; i <= a.size(); ++i)
        delete []matrice[i];
    delete[] matrice;
    return result;
}

float levenshteinPercent(const std::string &a, const std::string &b)
{
    if (a.empty() && b.empty())
        return 1.f;
    return 1 - (levenshtein(a, b) / std::max(a.size(), b.size()));
}

/**
 * Levenshtein Matrice Builder stuff
**/
LevenshteinMatrice_base::Builder::Builder()
{ }

LevenshteinMatrice_base::Builder::~Builder()
{ }

LevenshteinMatrice_base *LevenshteinMatrice_base::Builder::build(const JSonElement *a, const JSonElement *b) const
{
    const bool aIsContainer = ((dynamic_cast<const JSonContainer*>(a)) != nullptr);
    const bool bIsContainer = ((dynamic_cast<const JSonContainer*>(b)) != nullptr);

    if (aIsContainer && bIsContainer)
    {
        const size_t lenA = ((const JSonContainer*) a)->size();
        const size_t lenB = ((const JSonContainer*) b)->size();

        const JSonContainer::const_iterator aBegin = ((const JSonContainer*)a)->cbegin();
        const JSonContainer::const_iterator aEnd = ((const JSonContainer*)a)->cend();
        const JSonContainer::const_iterator bBegin = ((const JSonContainer*)b)->cbegin();
        const JSonContainer::const_iterator bEnd = ((const JSonContainer*)b)->cend();

        if (lenA < UCHAR_MAX && lenB < UCHAR_MAX)
            return new LevenshteinMatrice<unsigned char>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
        if (lenA < USHRT_MAX && lenB < USHRT_MAX)
            return new LevenshteinMatrice<unsigned short>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
        return new LevenshteinMatrice<unsigned int>(aBegin, aEnd, bBegin, bEnd, lenA, lenB);
    }
    else if (aIsContainer)
    {
        LevenshteinMatrice_manual *result = new LevenshteinMatrice_manual();
        result->_result = 2;
        return result->add(a, eLevenshteinOperator::rem)
            ->add(b, eLevenshteinOperator::add);
    }
    else if (bIsContainer)
    {
        LevenshteinMatrice_manual *result = new LevenshteinMatrice_manual();
        result->_result = 2;
        return result->add(b, eLevenshteinOperator::rem)
            ->add(a, eLevenshteinOperator::add);
    }
    else
    {
        const bool aIsObject = ((dynamic_cast<const JSonObjectEntry*>(a)) != nullptr);
        const bool bIsObject = ((dynamic_cast<const JSonObjectEntry*>(b)) != nullptr);
        float result = levenshteinPercent(a->stringify(), b->stringify());

        if (aIsObject && bIsObject) {
            result *= levenshteinPercent((*(const JSonObjectEntry&)(*a))->stringify(), (*(const JSonObjectEntry&)(*b))->stringify());
        }
        return new LevenshteinMatriceWithScore(result);
    }
}

/**
 * Manual matrice
**/
LevenshteinMatrice_manual *LevenshteinMatrice_manual::add(const JSonElement *a, eLevenshteinOperator b)
{
    operations[a] = b;
    return this;
}

void LevenshteinMatrice_manual::debug(std::ostream &out) const
{
    out << "(MANUAL - no data)" << std::endl;
}

size_t LevenshteinMatrice_manual::result() const
{
    return _result;
}

bool LevenshteinMatrice_manual::areSimilar() const
{
    return false;
}

/**
 * Score matrice
**/
LevenshteinMatriceWithScore::LevenshteinMatriceWithScore(float s)
{
    _result = s > LEVENSHTEIN_SENSIBILITY;
}

void LevenshteinMatriceWithScore::debug(std::ostream &out) const
{
    out << "Comparing two raw types gave " << (_result ? "=" : "!=") << std::endl;
}

size_t LevenshteinMatriceWithScore::result() const
{
    return _result ? 0 : 1;
}

bool LevenshteinMatriceWithScore::areSimilar() const
{
    return _result;
}

