#pragma once

#include <stdexcept>

template <typename T>
class Optional
{
    public:
        static Optional<T> of(T value)
        {
            Optional<T> result;
            result.exists = true;
            result.value = value;
            return result;
        }

        static Optional<T> _empty()
        {
            Optional<T> result;
            result.exists = false;
            return result;
        }

        /**
         * Check whether value exists or not
        **/
        bool absent() const
        {
            return !exists;
        }

        T orValue(T value) const
        {
            return exists ? this->value : value;
        }

        /**
         * return the element, or throw if absent
         * @throws std::logic_error
         * @return T value
        **/
        T get() const
        {
            if (!exists)
                throw std::logic_error("Optional does not contains value");
            return value;
        }

        T operator*() const
        {
            return get();
        }

        static Optional<T> empty;

    protected:
        T value;
        bool exists;

    private:
        Optional() {}
};

template <typename T> Optional<T> Optional<T>::empty = Optional<T>::_empty();

