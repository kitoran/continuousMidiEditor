#include "settings.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <linux/limits.h>
//#include <libconfig.h>
#include <stdlib.h>
#include "/home/n/library/stb/stb_ds.h"
extern const char* appName;
static char path[PATH_MAX+1] = {};
static char path_back[PATH_MAX+1] = {};
char* fileContent;

static struct Pair {
    char* key;
    char* value;
} * hmap = NULL;
typedef struct Pair Pair;
static void init() {
    static bool initialized = false;
    if(initialized) {
        return;
    }
    //return;
    char* home = getenv("HOME");
    assert(home);

    strncat(path, home, PATH_MAX);
    strncat(path, "/.config/", PATH_MAX);
    struct stat sb;
    if(stat(path, &sb) != 0 || (! S_ISDIR(sb.st_mode))) {
        abort();
    }
    strncat(path, appName, PATH_MAX);
    if(stat(path, &sb) != 0 || (! S_ISDIR(sb.st_mode))) {
        mkdir(path, 0777);
    }
    strncat(path, "/", PATH_MAX);
    strncat(path, appName, PATH_MAX);
    strncat(path, ".conf", PATH_MAX);
    snprintf(path_back, PATH_MAX+1,
             "%s~", path);
//    int fd
    stat(path, &sb);
    if(stat(path, &sb) != 0) {
        goto leave;
//        int fd = open(path, O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
//        close(fd);
    }
    fileContent = malloc(sb.st_size);
    FILE* f = fopen(path, "r");
    char* line;
    size_t len = 0;
    while(getline(&line, &len, f) > 0) {
        char* key; char* value;
        if(sscanf(line, " %ms = %ms ", &key, &value) > 0) {
            shput(hmap, key, value);
        }
    }
    fclose(f);

//    int index = 0;
//    while(index < sb.st_size) {

//    }


//    const char *str;

//    config_init(&cfg);
//    setting = config_root_setting(&cfg);

//    if(! config_read_file(&cfg, path))
//    {
//        fprintf(stderr, "Error while reading settings file.\n");
//        config_destroy(&cfg);
//        abort();
//    }
    leave:
    initialized = true;
}
static void save() {
    FILE* f = fopen(path_back, "w");
//    char* line;
//    int len = 0;
    for (int i=0; i < shlen(hmap); ++i) {
        fprintf(f, "%s = %s\n", hmap[i].key, hmap[i].value);
    }
    fclose(f);
    rename(path_back, path);
}
int loadInt(char *name, bool* success)
{
    init();
    int index = shgeti(hmap, name);
    if(index >= 0) {
        *success = true;
        char* s = hmap[index].value;
        int res;
        sscanf(s, " %d ", &res);
        return res;
    } else {
        *success = false;
        return 0;
    }
}

void saveInt(char *name, int value)
{
    init();
    int index = shgeti(hmap, name);
    if(index >= 0) {
        free(hmap[index].value);
    }
    char * string = malloc(20);
    snprintf(string, 20, "%d", value);
    shput(hmap, name, string);

    save();
    return;
}
