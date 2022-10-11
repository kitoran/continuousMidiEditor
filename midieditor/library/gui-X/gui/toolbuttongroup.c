#include "toolbuttongroup.h"
#include "stdbool.h"
#include "gui.h"
#include "gridlayout.h"
#include "persistent.h"
#include "stb_ds.h"
bool guiToolButtonGroup_(Painter* p, int* enumv, const char*const fileNames[], int enumSize)
{
    Point pos = getPos();

    if(!guiSameWindow(p)) return false;
    Grid* g;
    static struct {
        void* key;
        Grid* value;
    } *map = NULL;
    int index = hmgeti(map, enumv);
    if(index == -1) {
        g = malloc(sizeof(Grid));
        *g = allocateGrid(enumSize, 1, 0);
        g->gridStart = pos;
        hmput(map, enumv, g);
    } else {
        g = map[index].value;
    }
    g->gridStart = pos;
    pushGrid(g);

    bool res = false;
    bool noConsume;
    setCurrentGridPos(0,0);

    for(int i = 0; i < enumSize; i++) {
        if(resourseToolButtonA(p, fileNames[i], i == *enumv, &noConsume)) {
            *enumv = i;
            res = true;
            guiRedraw();
        }
        gridNextColumn(g);
    }

    Size size = {getGridWidth(g),
                 getGridHeight(g)};

    popGrid();
    feedbackSize(size);
    return res;
}
