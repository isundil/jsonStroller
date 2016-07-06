#pragma once

#include <fstream>
#include <ios>
#include <set>
#include <list>
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
        virtual void loop();

    protected:
        void init();
        void shutdown();
        /**
         * return false if bottom of screen is touched
        **/
        bool redraw();
        bool redraw(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxWidth, const JSonElement *, const JSonContainer *);
        bool readInput();
        void getScreenSize(std::pair<unsigned int, unsigned int> &, std::pair<int, int> &) const;
        static const JSonElement* findPrev(const JSonElement *);
        static const JSonElement* findNext(const JSonElement *);
        void checkSelection(const JSonElement *item, const JSonElement *parent, const std::pair<int, int>&);
        static unsigned int getNbLines(float nbChar, unsigned int maxWidth);
        unsigned int write(const int &x, const int &y, const JSonElement *item, unsigned int maxWidth, bool selected =false);
        unsigned int write(const int &x, const int &y, const std::string &item, unsigned int maxWidth, bool selected =false);
        unsigned int write(const int &x, const int &y, const char item, unsigned int maxWidth, bool selected =false);
        unsigned int write(const int &x, const int &y, const char *item, unsigned int maxWidth, bool selected =false);
        bool writeKey(const std::string &key, std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxWidth, bool selected);
        bool writeContainer(std::pair<int, int> &, const std::pair<unsigned int, unsigned int> &maxSize, const JSonContainer *);
        bool writeContent(std::pair<int, int> &cursor, const std::pair<unsigned int, unsigned int> &maxSize, const std::list<JSonElement *> * obj);

        std::set<const JSonContainer *> collapsed;

        const JSonElement *data, *selection;
        SCREEN *screen;
        FILE *screen_fd;
        bool breakLoop;
        int topleft;
        const unsigned int indentLevel;

        //FIXME optimize
        const JSonElement *select_up, *select_down;
        bool selectFound, selectIsLast, selectIsFirst;
};

