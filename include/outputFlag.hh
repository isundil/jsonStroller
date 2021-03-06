/**
 * outputFlag.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

enum eLevenshteinOperator: char
{
    add = '+',
    rem = '-',
    mod = '!',
    equ = '='
};

class OutputFlag
{
    public:
        OutputFlag(short mode =0);
        virtual ~OutputFlag();

        /**
         * get/set SELECTED byte
        **/
        bool selected() const;
        bool selected(bool v);

        /**
         * get/set SEARCH byte
        **/
        bool searched() const;
        bool searched(bool v);

        /**
         * get/set item's type
        **/
        char type() const;
        char type(char t);

        /**
         * get/set item's diff operation
        **/
        eLevenshteinOperator diffOp() const;
        eLevenshteinOperator diffOp(const eLevenshteinOperator &);

    protected:
        /**
         * item mode bitmask
        **/
        short mode;
        /**
         * item type
        **/
        char _type;

        eLevenshteinOperator diffOpt;

    public:
        static const short MODE_SELECTED = 1;
        static const short MODE_SEARCHED = 2;

        static const char TYPE_UNKNOWN;
        static const char TYPE_STRING;
        static const char TYPE_NUMBER;
        static const char TYPE_BOOL;
        static const char TYPE_OBJ;
        static const char TYPE_OBJKEY;
        static const char TYPE_ARR;
        static const char TYPE_NULL;

        static const char SPECIAL_NONE;
        static const char SPECIAL_SEARCH;
        static const char SPECIAL_ERROR;
        static const char SPECIAL_INPUTNAME;
        static const char SPECIAL_ACTIVEINPUTNAME;

        static const char DIFF_ADD;
        static const char DIFF_MOD;
        static const char DIFF_REM;
};

