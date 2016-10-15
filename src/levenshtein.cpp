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
        const JSonContainer::const_iterator bBegin = ((const JSonContainer*)b)->cbegin();

        LevenshteinMatrice *result = nullptr;

        if (lenA < UCHAR_MAX && lenB < UCHAR_MAX)
            result = LevenshteinMatrice::build<unsigned char>(aBegin, bBegin, lenA, lenB);
        else if (lenA < USHRT_MAX && lenB < USHRT_MAX)
            result = LevenshteinMatrice::build<unsigned short>(aBegin, bBegin, lenA, lenB);
        else
            result = LevenshteinMatrice::build<unsigned int>(aBegin, bBegin, lenA, lenB);
        result->addRoot((const JSonContainer *)a, (const JSonContainer *)b);
        return result;
    }
    else if (aIsContainer)
    {
        LevenshteinMatrice_manual *result = new LevenshteinMatrice_manual();
        result->_result = ((JSonContainer*)a)->size() +1; //TODO recursive number of all descendants
        return result->add(a, eLevenshteinOperator::rem)
            ->add(b, eLevenshteinOperator::add);
    }
    else if (bIsContainer)
    {
        LevenshteinMatrice_manual *result = new LevenshteinMatrice_manual();
        result->_result = ((JSonContainer*)b)->size() +1; //TODO recursive number of all descendants
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
        return new LevenshteinMatriceWithScore(result, a, b);
    }
}

eLevenshteinOperator LevenshteinMatrice_base::get(const JSonElement *e) const
{
    return operations.at(e);
}

const JSonElement *LevenshteinMatrice_base::getEquivalence(const JSonElement *) const
{ return nullptr; }

/**
 * base (generic) Matrice
**/
const std::map<const JSonElement*, eLevenshteinOperator> LevenshteinMatrice_base::path() const
{ return operations; }

std::map<const JSonElement*, const JSonElement *> LevenshteinMatrice_base::getEquivalences() const
{ return std::map<const JSonElement*, const JSonElement *>(); }

/**
 * Normal matrice
**/
LevenshteinMatrice::LevenshteinMatrice()
{ }

size_t LevenshteinMatrice::result() const
{ return levenDist; }

bool LevenshteinMatrice::areSimilar() const
{ return levenRelativeDist > LEVENSHTEIN_SENSIBILITY; }

const JSonElement *LevenshteinMatrice::getEquivalence(const JSonElement *e) const
{
    for (std::pair<const JSonElement *, const JSonElement *> it : equivalences)
    {
        if (it.first == e)
            return it.second;
        if (it.second == e)
            return it.first;
    }
    return nullptr;
}

std::map<const JSonElement*, const JSonElement *> LevenshteinMatrice::getEquivalences() const
{ return equivalences; }

/**
 * Manual matrice
**/
LevenshteinMatrice_manual *LevenshteinMatrice_manual::add(const JSonElement *a, eLevenshteinOperator b)
{
    operations[a] = b;
    return this;
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
LevenshteinMatriceWithScore::LevenshteinMatriceWithScore(float s, const JSonElement *a, const JSonElement *b)
{
    _result = s > LEVENSHTEIN_SENSIBILITY;
    if (_result)
    {
        equivalentA = a;
        equivalentB = b;
    }
    else
        equivalentA = equivalentB = nullptr;
}

const JSonElement * LevenshteinMatriceWithScore::getEquivalence(const JSonElement *a) const
{
    if (equivalentA && equivalentB && equivalentA == a)
        return equivalentB;
    return nullptr;
}

std::map<const JSonElement *, const JSonElement *> LevenshteinMatriceWithScore::getEquivalences() const
{
    std::map<const JSonElement*, const JSonElement *> res;
    if (equivalentA && equivalentB)
        res[equivalentA] = equivalentB;
    return res;
}

size_t LevenshteinMatriceWithScore::result() const
{
    return _result ? 0 : 1;
}

bool LevenshteinMatriceWithScore::areSimilar() const
{
    return _result;
}

