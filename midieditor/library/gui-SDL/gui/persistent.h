#ifndef PERSISTENT_H
#define PERSISTENT_H
#include <gui.h>


bool persistentNumberEdit_(Painter*p, int digits, int* number, char* name, bool *consume);
#define persistentNumberEdit(p, d, n, c) \
    persistentNumberEdit_(p, d, n, #n, c)

bool resourseToolButton(Painter*p, char* name, bool *consume);
bool standardResourseToolButton(Painter*p, char* name, bool* consume);
bool persistentComboBoxZT_(Painter*p, char const*const* elements, int *current, char* name);
#endif // PERSISTENT_H
