#pragma once

#include <fstream>
#include <ios>

class JSonElement;
template<class T> class Optional;

class CurseOutput
{
    public:
        CurseOutput(JSonElement *rootData);
        virtual ~CurseOutput();

        void run();

    private:
        typedef Optional<std::pair<Optional<const std::string>, const JSonElement *> > t_nextKey;

    protected:
        void init();
        void shutdown();
        void redraw();
        void redraw(std::pair<int, int> &, const std::pair<int, int>&, const JSonElement *);
        bool readInput();
        void getScreenSize(std::pair<int, int> &);
        static CurseOutput::t_nextKey findNext(const JSonElement *);
        void write(const int &x, const int &y, JSonElement *item);
        void write(const int &x, const int &y, const std::string &item);
        void writeKey(const std::string &key, std::pair<int, int> &cursor);

        const JSonElement *data, *selection;
        const JSonElement *select_up, *select_down;
        bool selectFound;
        bool selected;
        std::pair<std::pair<unsigned int, unsigned int>, const JSonElement *> topleft;
        const unsigned int indentLevel;
};

