/**
 * wrappedBuffer.hpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

#include <stdexcept>
#include <string.h>
#include <string>

class NotImplementedException: public std::logic_error
{
    public:
        NotImplementedException(): std::logic_error("Not implemented") {};
};

template <typename T, int SIZE =10>
class WrappedBuffer
{
    public:
        WrappedBuffer();
        virtual ~WrappedBuffer();

        /**
         * append item(s) to buffer
        **/
        virtual void put(T item);
        virtual void put(T items[], unsigned int count);
        /**
         * remove last item from buffer
        **/
        void pop_back();
        /**
         * empty the buffer
        **/
        void reset();

        /**
         * @return total size appended since instanciation or clear()
        **/
        unsigned int totalSize() const;
        /**
         * @return Current size (cannot be greater than SIZE
        **/
        unsigned int size() const;

        /**
         * @return basic_string representation of the buffer
        **/
        std::basic_string<T> toString() const;
        T* toArray(T arr[SIZE]) const;

    protected:
        T buffer[SIZE];
        int curR;
        int curW;
        unsigned int written;
};

template<typename T, int SIZE>
WrappedBuffer<T, SIZE>::WrappedBuffer(): curR(0), curW(-1)
{ }

template<typename T, int SIZE>
WrappedBuffer<T, SIZE>::~WrappedBuffer()
{ }

template<typename T, int SIZE>
void WrappedBuffer<T, SIZE>::reset()
{
    curR = 0;
    curW = -1;
    written = 0;
}

template<typename T, int SIZE>
unsigned int WrappedBuffer<T, SIZE>::totalSize() const
{ return written; }

template<typename T, int SIZE>
void WrappedBuffer<T, SIZE>::put(T item)
{
    written++;
    if (curW +1 == SIZE)
    {
        curR = 1;
        curW = 0;
        buffer[0] = item;
    }
    else if (curW == -1)
    {
        curR = SIZE;
        buffer[curW = 0] = item;
    }
    else
    {
        buffer[++curW] = item;
        if (curR == curW)
        {
            if (++curR > SIZE)
                curR = 0;
        }
    }
}

template<typename T, int SIZE>
void WrappedBuffer<T, SIZE>::put(T items[], unsigned int count)
{
    unsigned int newSize = size() + count;

    if (!count)
        return;
    written += count;
    while (count > SIZE)
    {
        count -= SIZE;
        items += SIZE;
    }
    if (curW + count >= SIZE)
    {
        if (curW +1 != SIZE)
        {
            memcpy(&buffer[curW +1], items, sizeof(T) * (SIZE - curW -1));
            items += (SIZE - curW -1);
            count -= (SIZE - curW -1);
        }
        curW = -1;
    }
    memcpy(&buffer[curW +1], items, sizeof(T) * count);
    curW += count;
    if (curW == SIZE)
    {
        curW = 0;
        curR = 1;
    }
    else if (newSize >= SIZE)
        curR = (curW +1) % SIZE;
}

template<typename T, int SIZE>
void WrappedBuffer<T, SIZE>::pop_back()
{
    unsigned int oldSize = size();
    written--;
    if (oldSize == 0)
        return;
    else if (oldSize == 1)
    {
        curW = -1;
        curR = 0;
    }
    else if (--curW < 0)
    {
        curW += SIZE;
    }
}

template<typename T, int SIZE>
unsigned int WrappedBuffer<T, SIZE>::size() const
{
    if (curW == -1)
        return 0;
    return (curR > curW) ? (SIZE - curR + curW +1) : (curW - curR +1);
}

template<typename T, int SIZE>
std::basic_string<T> WrappedBuffer<T, SIZE>::toString() const
{
    const unsigned int size = this->size();
    std::basic_string<T> result;
    if (!size)
        return result;
    result.reserve(size);
    const int from = (curR == SIZE) ? 0 : curR;
    int i = 0;
    unsigned int j =0;

    for (i = from; (curW >= from && i <= curW) || (curW < from && i < SIZE); ++i)
    {
        j++;
        result += buffer[i];
    }
    i = 0;
    while (i <= curW && j < size)
    {
        result += buffer[i++];
        j++;
    }
    return result;
}

template<typename T, int SIZE>
T* WrappedBuffer<T, SIZE>::toArray(T arr[SIZE]) const
{
    throw NotImplementedException();
    if (!arr)
        arr = new T[SIZE]();
    //TODO
    return arr;
}

