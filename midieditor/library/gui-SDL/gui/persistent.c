#include "persistent.h"
#include "settings.h"
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
        SDL_Surface* value;
    } *map = NULL;

    SDL_Surface* image;
    int index = shgeti(map, name);
    if(index == -1) {
        image = loadImageZT(GUI_RESOURCE_PATH, name);
        shput(map, name, image);
    } else {
        image = map[index].value;
    }
    return guiToolButton(p, image, consume);
}
bool resourseToolButton(Painter*p, char* name, bool* consume) {
    static struct {
        char* key;
        SDL_Surface* value;
    } *map = NULL;

    SDL_Surface* image;
    int index = shgeti(map, name);
//    fprintf(stderr, "%d, index", index);
    if(index == -1) {
        image = loadLocalImageZT(name);
        shput(map, name, image);
    } else {
        image = map[index].value;
    }
    return guiToolButton(p, image, consume);
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
