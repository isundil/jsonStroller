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

        const std::map<const JSonElement*, eLevenshteinOperator> path() const;
        virtual size_t result() const =0;

        virtual void debug(std::ostream &out) const =0;

    public:
        class Builder
        {
            public:
                Builder();
                ~Builder();

                const LevenshteinMatrice_base *build(const JSonElement *a, const JSonElement *b) const;
        };

    protected:
        std::map<const JSonElement*, eLevenshteinOperator> operations;
};

class LevenshteinMatrice_manual: public LevenshteinMatrice_base
{
    public:
        LevenshteinMatrice_manual *add(const JSonElement*, eLevenshteinOperator);
        size_t result() const;

        void debug(std::ostream &out) const;

    public:
        size_t _result;
};

template<typename T>
class LevenshteinMatrice: public LevenshteinMatrice_base
{
    public:
        LevenshteinMatrice(const JSonContainer::const_iterator aBegin, const JSonContainer::const_iterator aEnd,
                const JSonContainer::const_iterator bBegin, const JSonContainer::const_iterator bEnd,
                size_t n, size_t m)
        {
            size_t i, j;
            JSonContainer::const_iterator a = aBegin;
            JSonContainer::const_iterator b;

            this->n = n;
            this->m = m;
            this->matrice = new T*[n +1]();
            this->subMatrice = new LevenshteinMatrice_base**[n +1]();

            matrice[0] = new T[m +1];
            for (i =1; i <= m; ++i)
                matrice[0][i] = i;

            for (i=1; i <= n; ++i)
            {
                matrice[i] = new T[m +1];
                matrice[i][0] = i;
            }

            for (i=0; i <= n; ++i)
            {
                subMatrice[i] = new LevenshteinMatrice_base*[m +1];
                for (size_t j=0; j <= m; ++j)
                    subMatrice[i][j] = nullptr;
            }

            for (i =1; a != aEnd; ++i, ++a)
            {
                b = bBegin;
                for (j =1; b != bEnd; ++j, ++b)
                {
                    //TODO compute submatrice
                    /*
                    matrice[i][j] = std::min(std::min(
                        get(i -1, j) +1,
                        get(i, j -1) +1),
                        get(i -1, j -1) + ((levenshteinCompare(*a, *b) > LEVENSHTEIN_SENSIBILITY) ? 0 : 1)); // TODO set submatrice
                    */
                    matrice[i][j] = std::min(
                        get(i -1, j) +1,
                        get(i, j -1) +1);
                }
            }
        };

        ~LevenshteinMatrice()
        {
            for (size_t i=0; i <= n; ++i)
            {
                delete []matrice[i];
                for (size_t j=0; j <= m; ++j)
                    if (subMatrice[i][j])
                        delete subMatrice[i][j];
                delete []subMatrice[i];
            }
            delete []matrice;
            delete []subMatrice;
        };

        void prune()
        {
            //TODO
        }

        T get(size_t a, size_t b) const
        {
            return matrice[a][b];
        };

        std::list<eLevenshteinOperator> shortestPath() const
        {
            std::list<eLevenshteinOperator> result;

            size_t i = n;
            size_t j = m;

            while (i || j)
            {
                if (i && (!j || matrice[i][j] > matrice[i-1][j]))
                {
                    result.push_front(eLevenshteinOperator::add);
                    --i;
                }
                else if (j && (!i || matrice[i][j] > matrice[i][j -1]))
                {
                    result.push_front(eLevenshteinOperator::rem);
                    --j;
                }
                else if (i && j)
                {
                    result.push_front(matrice[i][j] == matrice[i-1][j-1] ? eLevenshteinOperator::equ : eLevenshteinOperator::mod);
                    --i;
                    --j;
                }
                else if (i)
                {
                    result.push_front(eLevenshteinOperator::add);
                    --i;
                }
                else if (j)
                {
                    result.push_front(eLevenshteinOperator::rem);
                    --j;
                }
            }
            return result;
        }

        void debug(std::ostream &o) const
        {
            for (size_t i =0; i <= n; ++i)
            {
                for (size_t j=0; j <= m; ++j)
                    o << (int) (matrice[n][m]) << '\t';
                o << std::endl;
            }
        }

        size_t result() const
        {
            return (size_t) matrice[n][m];
        };

    private:
        T **matrice;
        /**
         * Usefull only on `modify' operation
        **/
        LevenshteinMatrice_base ***subMatrice;

        size_t n;
        size_t m;
};

