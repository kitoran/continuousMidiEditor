#include "gridlayout.h"
#include <stdio.h>
//#include <assert.h>
#include <stdlib.h>
#include <string.h>

Grid allocateGrid(int cols, int rows, int spacing) {
    Grid r = {.gridWidths = malloc(4*cols), .gridWidthsLen = cols,
              .gridHeights = malloc(4*rows), .gridHeightsLen = rows,
              .currentX = 0, .currentY = 0, .gridStart = {0,0}, .spacing=spacing};
    memset(r.gridWidths, 0, 4*cols);
    memset(r.gridHeights, 0, 4*rows);
    return r;
}
#define assert(a, msg, ...) if(!(a)) { \
    fprintf(stderr, "%s: " msg, __PRETTY_FUNCTION__, ##__VA_ARGS__); \
    abort(); \
}
//fprintf(stderr, msg, ##__VA_ARGS__);
void setCurrentGridPos(int row, int column) {
    Grid* g = topGrid();
    assert(row < g->gridHeightsLen, "row too big")
    assert(column < g->gridWidthsLen, "col too big")
    g->currentX = column;
    g->currentY = row;
}
int getGridHeight(Grid* g)
{
    int y = 0;
    for(int i = 0; i < g->gridHeightsLen; i++) {
        if(g->gridHeights[i])
            y += g->gridHeights[i]+g->spacing;
    }
    return y+g->spacing;
}

int getGridWidth(Grid *g)
{
    int x = 0;
    for(int i = 0; i < g->gridWidthsLen; i++) {
        if(g->gridWidths[i])
            x += g->gridWidths[i]+g->spacing;
    }
    return x+g->spacing;
}

int getGridBottom(Grid *g);
void gridNextRow()
{
    Grid* g = topGrid();
//    assert(g->gridWidthsLenvsdv
    g->currentY++;
}

void gridNextColumn()
{
    Grid* g = topGrid();
    g->currentY = 0;
    g->currentX++;
}

Grid* stack[10];
size_t stackTop = 0;

void pushGrid(Grid *g)
{
    assert(stackTop < ELEMS(stack), "pushing grid, stackTop is %lud, elems is %lud, "
           "cond is %d, !cond id %d", stackTop, ELEMS(stack),
           stackTop < ELEMS(stack), !(stackTop < ELEMS(stack)))
    stack[stackTop] = g;
    stackTop++;
}

void popGrid()
{
    assert(stackTop > 0, "stack is empty, trying to pop")
    stack[stackTop] = NULL;
    stackTop--;
}

Point gridGetPos() {
//    assert(stackTop > 0, "stack is empty, trying to use")
    Grid* g = topGrid();
    assert(g->currentX < g->gridWidthsLen, "%d, %d", g->currentX, g->gridWidthsLen)
    assert(g->currentY < g->gridHeightsLen,"")
    int x = g->gridStart.x, y = g->gridStart.y;
    for(int i = 0; i < g->currentX; i++) {
        if(g->gridWidths[i])
            x += g->gridWidths[i]+g->spacing;
    }
    for(int i = 0; i < g->currentY; i++) {
        if(g->gridHeights[i])
            y += g->gridHeights[i]+g->spacing;
    }
    Point res = {x, y};
    return res;
}

void gridFeedbackSize(Size s) {
    assert(stackTop > 0, "stack is empty, trying to pop")
    Grid* g = stack[stackTop-1];
    assert(g->currentX < g->gridWidthsLen, "%d, %d", g->currentX, g->gridWidthsLen)
    assert(g->currentY < g->gridHeightsLen,"")
    Grid* fw3ehbsergser = g;
    if(s.height > 1000) {
        fprintf(stderr, "deep %p", fw3ehbsergser);
    }
    if(g->gridWidths[g->currentX] < (int)s.width) {
        guiRedraw();
        g->gridWidths[g->currentX] = (int)s.width;
    }
    if(g->gridHeights[g->currentY] < (int)s.height) {
        guiRedraw();
        g->gridHeights[g->currentY] = (int)s.height;
    }
}

Grid *topGrid()
{
    assert(stackTop > 0, "stack is empty, trying to get top")
    return stack[stackTop-1];
}
