#ifndef PERSISTENT_H
#define PERSISTENT_H
#include <gui.h>


bool persistentNumberEdit_(Painter*p, int digits, int* number, char* name, bool *consume);
#define persistentNumberEdit(p, d, n, c) \
    persistentNumberEdit_(p, d, (int*)&n, #n, c)
bool resourseToolButtonA(Painter*p, const char* name, bool active, bool* consume);
bool resourseToolButton(Painter*p, const char* name, bool *consume);
bool standardResourseToolButton(Painter*p, char* name, bool* consume);
bool persistentComboBoxZT_(Painter*p, char const*const* elements, int *current, char* name);
#define persistentEnumComboBox(enum, p, c) \
    persistentComboBoxZT_(p, enum ## Names, &c, #c)
bool persistentToolButtonGroup_(Painter *p, int *v, const char*const* filenames, int enumSize, char* saveName);
#define persistentToolButtonGroup(p, e, c) \
    persistentToolButtonGroup_(p, (int*)&c, e ## Filenames, e## Size, #c)

#endif // PERSISTENT_H
