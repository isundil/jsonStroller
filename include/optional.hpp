#pragma once

template<class T>
class Optional
{
    public:
        Optional(): _empty(true)
        { };

        Optional(T v): _empty(false), _value(v)
        { }

        T value() const
        { return _value; }

        bool isPresent() const
        { return !_empty; }

        bool isAbsent() const
        { return _empty; }

    static Optional<T> empty()
        { return Optional<T>(); }

    private:
        bool _empty;
        T _value;
};

