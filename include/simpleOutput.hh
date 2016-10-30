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

        inline void writeObjectEntry(const JSonObjectEntry *, bool);
        inline void writePrimitive(const AJSonPrimitive *, bool);
        inline void writeContainer(const JSonContainer *, bool);
        inline void write(const JSonElement *, bool prependComma);

    private:
        std::ostream &out;
        const Params &params;
        unsigned short indent;
};

