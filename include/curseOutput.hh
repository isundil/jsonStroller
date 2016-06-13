#pragma once

class JSonElement;

class CurseOutput
{
    public:
        CurseOutput(JSonElement *rootData);
        virtual ~CurseOutput();

        void run();

    protected:
        void init();
        void shutdown();
        void redraw();
        void redraw(std::pair<int, int> &, const std::pair<int, int>&, const JSonElement *);
        bool readInput();
        void getScreenSize(std::pair<int, int> &);
        void write(const int &x, const int &y, JSonElement *item);
        void write(const int &x, const int &y, const std::string &item);

        unsigned int offset_x;
        unsigned int offset_y;

        const JSonElement *data;
        const unsigned int indentLevel;
};

