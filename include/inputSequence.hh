#pragma once

#include <string>

class InputSequence
{
    private:
        InputSequence();
        InputSequence(const InputSequence &);

    public:
        ~InputSequence();

        static InputSequence read();

        const std::string &key() const;

    protected:
        std::string seq;
};

