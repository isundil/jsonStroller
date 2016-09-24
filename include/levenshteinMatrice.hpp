#pragma once

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
        virtual void prune() =0;
};

template<typename T>
class LevenshteinMatrice: public LevenshteinMatrice_base
{
    public:
        LevenshteinMatrice(size_t n, size_t m)
        {
            this->n = n;
            this->m = m;
            this->matrice = new T*[n +1]();
            this->subMatrice = new LevenshteinMatrice_base**[n +1]();

            matrice[0] = new T[m +1];
            for (size_t i =1; i <= m; ++i)
                matrice[0][i] = i;

            for (size_t i=1; i <= n; ++i)
            {
                matrice[i] = new T[m +1];
                matrice[i][0] = i;
            }

            for (size_t i=0; i <= n; ++i)
            {
                subMatrice[i] = new LevenshteinMatrice_base*[m +1];
                for (size_t j=0; j <= m; ++j)
                    subMatrice[i][j] = nullptr;
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

        void set(size_t a, size_t b, T value, LevenshteinMatrice_base *subMatrice =nullptr)
        {
            matrice[a][b] = value;
            this->subMatrice[a][b] = subMatrice;
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

        T result() const
        {
            return matrice[n][m];
        };

    private:
        T **matrice;
        LevenshteinMatrice_base ***subMatrice;

        size_t n;
        size_t m;
};

