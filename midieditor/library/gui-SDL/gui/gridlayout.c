#include "gridlayout.h"
#include <stdio.h>
int gridWidths[200] = {};
int gridHeights[200] = {};
int currentX = 0;
int currentY = 0;
Point gridStart ={0,0};

Point gridGetPos() {
    int x = gridStart.x, y = gridStart.y;
    for(int i = 0; i < currentX; i++) {
        if(gridWidths[i])
            x += gridWidths[i]+5;
    }
    for(int i = 0; i < currentY; i++) {
        if(gridHeights[i])
            y += gridHeights[i]+5;
    }
    Point res = {x, y};
    return res;
}

void gridFeedbackSize(Size s) {
    if(s.height > 1000) {
        fprintf(stderr, "deep");
    }
    if(gridWidths[currentX] < (int)s.width) {
        guiRedraw();
        gridWidths[currentX] = (int)s.width;
    }
    if(gridHeights[currentY] < (int)s.height) {
        guiRedraw();
        gridHeights[currentY] = (int)s.height;
    }
}

void setCurrentGridPos(int row, int column) {
    currentX = column;
    currentY = row;
}

int getGridBottom()
{
    int x = gridStart.x;
    for(int i = 0; i < 200; i++) {
        if(gridHeights[i])
            x += gridHeights[i]+5;
    }
    return x;
}

void gridNextRow()
{
    currentY++;
}

void gridNextColumn()
{
    currentY = 0;
    currentX++;
}
