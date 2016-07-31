/**
 * outputFlag.hh for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#pragma once

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
         * get/set item's type
        **/
        char type() const;
        char type(char t);

    protected:
        /**
         * item mode bitmask
        **/
        short mode;
        /**
         * item type
        **/
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
        static const char SPECIAL_ERROR;
};

