#pragma once

#include <iostream>
#include <map>
#include "jsonContainer.hh"
#include "levenshtein.hpp"

enum eLevenshteinOperator: char
{
    add = '+',
    rem = '-',
    mod = '!',
    equ = '='
};

class LevenshteinMatrice_base
{
    public:
        virtual ~LevenshteinMatrice_base() {}

        virtual const std::map<const JSonElement*, eLevenshteinOperator> path() const;
        virtual size_t result() const =0;
        virtual bool areSimilar() const =0;

        eLevenshteinOperator get(const JSonElement *) const;
        virtual std::map<const JSonElement *, const JSonElement *> getEquivalences() const;
        virtual const JSonElement * getEquivalence(const JSonElement *) const;

    public:
        class Builder
        {
            public:
                Builder();
                ~Builder();

                LevenshteinMatrice_base *build(const JSonElement *a, const JSonElement *b) const;
        };

    protected:
        std::map<const JSonElement*, eLevenshteinOperator> operations;
};

class LevenshteinMatrice_manual: public LevenshteinMatrice_base
{
    public:
        LevenshteinMatrice_manual *add(const JSonElement*, eLevenshteinOperator);
        size_t result() const;
        bool areSimilar() const;

    public:
        size_t _result;
};

class LevenshteinMatriceWithScore: public LevenshteinMatrice_base
{
    public:
        LevenshteinMatriceWithScore(float score, const JSonElement *a, const JSonElement *b);

        std::map<const JSonElement *, const JSonElement *> getEquivalences() const;
        virtual const JSonElement * getEquivalence(const JSonElement *) const;

        size_t result() const;
        bool areSimilar() const;

    private:
        bool _result;
        const JSonElement *equivalentA, *equivalentB;
};

class LevenshteinMatrice: public LevenshteinMatrice_base
{
    public:
        template<typename T>
        static LevenshteinMatrice *build(const JSonContainer::const_iterator aBegin, const JSonContainer::const_iterator bBegin,
                const size_t n, const size_t m)
        {
            LevenshteinMatrice *result = new LevenshteinMatrice();
            size_t i, j;
            JSonContainer::const_iterator a = aBegin;
            JSonContainer::const_iterator b;
            LevenshteinMatrice_base::Builder matriceBuilder;

            T **matrice = new T*[n +1]();
            LevenshteinMatrice_base ***subMatrice = new LevenshteinMatrice_base**[n]();

            matrice[0] = new T[m +1];
            for (i =0; i <= m; ++i)
                matrice[0][i] = i;

            for (i=1; i <= n; ++i)
            {
                matrice[i] = new T[m +1];
                matrice[i][0] = i;
                subMatrice[i -1] = new LevenshteinMatrice_base*[m];
                for (size_t j=0; j < m; ++j)
                    subMatrice[i -1][j] = nullptr;
            }

            for (i =1; i <= n; ++i, ++a)
            {
                b = bBegin;
                for (j =1; j <= m; ++j, ++b)
                {
                    LevenshteinMatrice_base *_subMatrice = matriceBuilder.build(*a, *b);
                    if (_subMatrice != nullptr)
                    {
                        const T chCost = matrice[i -1][j -1] + (_subMatrice->areSimilar() ? 0 : _subMatrice->result());

                        if (chCost <= matrice[i -1][j] +1 &&
                                chCost <= matrice[i][j -1] +1)
                        {
                            matrice[i][j] = chCost;
                            subMatrice[i -1][j -1] = _subMatrice;
                            continue;
                        }
                        delete _subMatrice;
                    } // Change is not worth, consider adding/removing
                    matrice[i][j] = std::min(matrice[i -1][j], matrice[i][j -1]) +1;
                }
            }

            result->levenDist = matrice[n][m];
            result->levenRelativeDist = 1 -(matrice[n][m] / std::max(n, m));
            result->shortestPath<T>(matrice, subMatrice, n, m, --a, --b);
            cleanMatrice(matrice, subMatrice, n, m);
            return result;
        };

        template<typename T>
        static void cleanMatrice(T **matrice, LevenshteinMatrice_base ***subMatrice, const size_t &n, const size_t &m)
        {
            for (size_t i=0; i <= n; ++i)
            {
                delete []matrice[i];
                if (i != n)
                {
                    for (size_t j=0; j < m; ++j)
                        if (subMatrice[i][j])
                            delete subMatrice[i][j];
                    delete []subMatrice[i];
                }
            }
            delete []matrice;
            delete []subMatrice;
        };

        void addRoot(const JSonContainer *a, const JSonContainer *b)
        {
            if (levenRelativeDist < LEVENSHTEIN_SENSIBILITY)
            {
                operations[a] = eLevenshteinOperator::add;
                operations[b] = eLevenshteinOperator::add;
            }
            else
            {
                operations[a] = operations[b] = levenRelativeDist < 1.f ? eLevenshteinOperator::mod : eLevenshteinOperator::equ;
                equivalences[a] = b;
                equivalences[b] = a;
            }
        }

        std::map<const JSonElement*, const JSonElement *> getEquivalences() const;

        size_t result() const;
        bool areSimilar() const;
        const JSonElement *getEquivalence(const JSonElement *) const;

    private:
        template<typename T>
        void shortestPath(T **matrice,
                LevenshteinMatrice_base ***subMatrice,
                size_t _i, size_t _j,
                JSonContainer::const_iterator i, JSonContainer::const_iterator j)
        {
            while (_i || _j)
            {
                if (_i && (!_j || matrice[_i][_j] > matrice[_i-1][_j]))
                {
                    operations[*i] = eLevenshteinOperator::add;
                    --i;
                    --_i;
                }
                else if (_j && (!_i || matrice[_i][_j] > matrice[_i][_j -1]))
                {
                    operations[*j] = eLevenshteinOperator::add;
                    --j;
                    --_j;
                }
                else if (_i && _j)
                {
                    eLevenshteinOperator op =
                        matrice[_i][_j] == matrice[_i -1][_j -1] ?
                        eLevenshteinOperator::equ :
                        eLevenshteinOperator::mod;
                    operations[*i] = operations[*j] = op;
                    for (std::pair<const JSonElement *, eLevenshteinOperator> e : subMatrice[_i -1][_j -1]->path())
                        operations[e.first] = e.second;
                    for (std::pair<const JSonElement *, const JSonElement *> e : (subMatrice[_i -1][_j -1])->getEquivalences())
                        equivalences[e.first] = e.second;
                    equivalences[*i] = *j;
                    --i;
                    --j;
                    --_i;
                    --_j;
                }
                else if (_i)
                {
                    operations[*i] = eLevenshteinOperator::add;
                    --i;
                    --_i;
                }
                else if (_j)
                {
                    operations[*j] = eLevenshteinOperator::add;
                    --j;
                    --_j;
                }
            }
        }


    private:
        LevenshteinMatrice();

        size_t levenDist;
        float levenRelativeDist;
        std::map<const JSonElement*, const JSonElement *> equivalences;
};

