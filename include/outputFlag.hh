#pragma once

class OutputFlag
{
    public:
        OutputFlag(short mode =0);
        virtual ~OutputFlag();

        bool selected() const;
        bool selected(bool v);

        char type() const;
        char type(char t);

    protected:
        short mode;
        char _type;

    public:
        static const short MODE_SELECTED = 1;

        static const char TYPE_UNKNOWN;
        static const char TYPE_STRING;
        static const char TYPE_NUMBER;
        static const char TYPE_BOOL;
        static const char TYPE_OBJ;
        static const char TYPE_OBJKEY;
        static const char TYPE_ARR;

        static const char SPECIAL_NONE;
        static const char SPECIAL_SEARCH;
};

