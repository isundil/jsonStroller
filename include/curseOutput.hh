#pragma once

#include <fstream>
#include <ios>
#include <set>
#include <ncurses.h>

class JSonElement;
class JSonContainer;
class JSonArray;
class JSonObject;
template<class T> class Optional;

class CurseOutput
{
    public:
        CurseOutput(JSonElement *rootData);
        virtual ~CurseOutput();

        void run();
        bool onsig(int signo);

    private:
        typedef Optional<std::pair<Optional<const std::string>, const JSonElement *> > t_nextKey;
        virtual void loop();

    protected:
        void init();
        void shutdown();
        /**
         * return false if bottom of screen is touched
        **/
        bool redraw();
        bool redraw(std::pair<int, int> &, const std::pair<int, int>&, const JSonElement *, const JSonContainer *);
        bool readInput();
        void getScreenSize(std::pair<int, int> &, std::pair<int, int> &);
        static CurseOutput::t_nextKey findNext(const JSonElement *);
        void checkSelection(const JSonElement *item, const JSonElement *parent, const std::pair<int, int>&);
        void write(const int &x, const int &y, const JSonElement *item, bool selected =false);
        void write(const int &x, const int &y, const std::string &item, bool selected =false);
        void write(const int &x, const int &y, const char item, bool selected =false);
        void write(const int &x, const int &y, const char *item, bool selected =false);
        bool writeKey(const std::string &key, std::pair<int, int> &cursor, const int maxHeight, bool selected);
        bool writeContainer(std::pair<int, int> &, const std::pair<int, int>&, const JSonContainer *);
        bool writeContent(std::pair<int, int> &cursor, const std::pair<int, int> &maxSize, const JSonArray * obj);
        bool writeContent(std::pair<int, int> &cursor, const std::pair<int, int> &maxSize, const JSonObject * obj);

        std::set<const JSonContainer *> collapsed;

        const JSonElement *data, *selection;
        SCREEN *screen;
        FILE *screen_fd;
        bool breakLoop;
        int topleft;
        const unsigned int indentLevel;

        //FIXME optimize
        const JSonElement *select_up, *select_down, *select_parent;
        bool selectFound, selectIsLast, selectIsFirst;
};

