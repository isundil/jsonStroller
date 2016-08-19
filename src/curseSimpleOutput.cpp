/**
 * curseOutput.cpp for jsonstroller
 *
 * Author: isundil <isundill@gmail.com>
**/

#include "curseSimpleOutput.hh"
#include "jsonContainer.hh"

CurseSimpleOutput::CurseSimpleOutput(const Params &p): CurseOutput(p)
{ }

CurseSimpleOutput::~CurseSimpleOutput()
{ }

void CurseSimpleOutput::run(JSonElement *root)
{
    selection = data = root;
    loop();
}

bool CurseSimpleOutput::redraw()
{
    const std::pair<unsigned int, unsigned int> screenSize = getScreenSize();
    std::pair<int, int> cursor(0, 0);
    /**
     * Will be true if the json's last item is visible
    **/
    bool result;

    select_up = select_down = nullptr;
    selectFound = selectIsLast = false;
    clear();
    try {
        result = CurseOutput::redraw(cursor, screenSize, data);
    }
    catch (SelectionOutOfRange &e)
    {
        return false;
    }
    if (!result && !selectFound)
    {
        scrollTop++;
        return false;
    }
    if (!result && !select_down)
        selectIsLast = true;
    if (!select_down)
    {
        const JSonContainer *pselect = dynamic_cast<const JSonContainer*>(selection);
        if (pselect && !pselect->empty())
            select_down = *(pselect->cbegin());
        else
        {
            const JSonElement *next = selection->findNext();
            select_down = next ? next : selection;
        }
    }
    if (!select_up)
        select_up = selection;
    refresh();
    return true;
}

