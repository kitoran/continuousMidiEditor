#ifndef GRIDLAYOUT_H
#define GRIDLAYOUT_H
// For now this is the single grid layout
// if i ever need different layouts in one gui i will change this
#include "gui.h"

extern Point gridStart;
Point gridGetPos();
void gridFeedbackSize(Size s);
void setCurrentGridPos(int row, int column);
void gridNextRow();
void gridNextColumn();
int getGridBottom();


#endif // GRIDLAYOUT_H
