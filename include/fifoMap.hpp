#pragma once

#include <stdexcept>
#include <utility>
#include <deque>

template <class K, class V>
class fifoMap: public std::deque<std::pair<K, V>>
{
    public:
        bool contains(const K &) const;
        V &operator[](const K&);
};

template<class K, class V>
bool fifoMap<K, V>::contains(const K &k) const
{
    for (std::pair<K, V> i: *this)
        if (i.first == k)
            return true;
    return false;
}

template<class K, class V>
V &fifoMap<K, V>::operator[](const K &k)
{
    for (std::pair<K, V> &i: *this)
        if (i.first == k)
            return i.second;
    this->push_back(std::pair<K, V>(k, V()));
    std::pair<K, V> &i = this->back();
    return i.second;
}

