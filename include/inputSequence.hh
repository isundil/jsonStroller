#pragma once

class InputSequence
{
    private:
        InputSequence();
        InputSequence(const InputSequence &);

    public:
        ~InputSequence();

        static InputSequence read();

        int key() const;

    protected:
        int _key;
};

