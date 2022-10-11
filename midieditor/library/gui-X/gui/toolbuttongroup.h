#ifndef TOOLBUTTONGROUP_H
#define TOOLBUTTONGROUP_H
struct Painter;
_Bool guiToolButtonGroup_(struct Painter* p, int* enumv, const char*const enumNames[], int enumSize);
//_Bool guiToolButtonGroup(struct Painter* p, int* enumv, char* enumNames[], int enumSize);
#define guiToolButtonGroup(p, e, v) \
    guiToolButtonGroup_(p, (int*)v, e ## Filenames, e## Size)
#endif // TOOLBUTTONGROUP_H
