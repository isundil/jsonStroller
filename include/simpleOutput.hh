#pragma once

#include <iostream>

class JSonElement;
class Params;

class SimpleOutput
{
    public:
        static void display(std::ostream &out, const JSonElement *root, const Params &params);

    private:
        SimpleOutput(std::ostream &output, const Params &p);

        std::string getIndent() const;

        inline void writeObjectEntry(const JSonObjectEntry *);
        inline void writePrimitive(const AJSonPrimitive *);
        inline void writeContainer(const JSonContainer *);
        inline void write(const JSonElement *);

        void indent_inc(int i =1);

    private:
        std::ostream &out;
        const Params &params;
        unsigned short indent;
};

