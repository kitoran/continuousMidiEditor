#include "persistent.h"
#include "settings.h"
#include "toolbuttongroup.h"
#include "loadImage.h"
#include "stb_ds.h"
#include "stdio.h"

typedef struct Unit {} Unit;
static Unit unit;
bool persistentNumberEdit_(Painter*p, int digits, int* number, char* name, bool* consume) {
    static struct {
        char* key;
        Unit value;
    } *map = NULL;

    int index = shgeti(map, name);
    if(index == -1) {
        bool success;
        int value = loadInt(name, &success);
        if(success) {
            *number = value;
        }
        shput(map, name, unit);
    }
    if(guiNumberEdit(p, digits, number, consume)) {
        saveInt(name, *number);
        return true;
    }
    return false;
}
bool standardResourseToolButton(Painter*p, char* name, bool* consume) {
    static struct {
        char* key;
        IMAGE* value;
    } *map = NULL;

    IMAGE* image;
    int index = shgeti(map, name);
    if(index == -1) {
        image = loadImageZT(GUI_RESOURCE_PATH, name);
        shput(map, name, image);
    } else {
        image = map[index].value;
    }
    return guiToolButton(p, image, consume);
}
bool resourseToolButton(Painter*p, const char* name, bool* consume) {
   return resourseToolButtonA(p, name, false, consume);
}
bool resourseToolButtonA(Painter*p, const char* name, bool active, bool* consume) {
    static struct {
        char* key;
        IMAGE* value;
    } *map = NULL;

    IMAGE* image;
    int index = shgeti(map, name);
//    fprintf(stderr, "%d, index", index);
    if(index == -1) {
        image = loadLocalImageZT(name);
        shput(map, name, image);
    } else {
        image = map[index].value;
    }
    return guiToolButtonA(p, image, active, consume);
}

bool persistentComboBoxZT_(Painter *p, const char * const *elements, int *current, char *name)
{
    static struct {
        char* key;
        Unit value;
    } *map = NULL;

    int index = shgeti(map, name);
    if(index == -1) {
        bool success;
        int value = loadInt(name, &success);
        if(success) {
            *current = value;
        }
        shput(map, name, unit);
    }
    if(guiComboBoxZT(p, elements, current)) {
        saveInt(name, *current);
        return true;
    }
    return false;
}
bool persistentToolButtonGroup_(Painter *p, int *v, const char*const*  filenames, int enumSize, char* saveName) {
    static struct {
        char* key;
        Unit value;
    } *map = NULL;

    int index = shgeti(map, saveName);
    if(index == -1) {
        bool success;
        int value = loadInt(saveName, &success);
        if(success) {
            *v = value;
        }
        shput(map, saveName, unit);
    }
    if(guiToolButtonGroup_(p,v,filenames,enumSize)) {
        saveInt(saveName, *v);
        return true;
    }
    return false;
}
