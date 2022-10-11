#include "newFile.h"


#include <sys/types.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <linux/limits.h>
// this code is BAD and SHOULD NOT BE USED however i CANT BE BOTHERED cause i'm DOING SOMETHING ELSE

// but tbh part of it is that c strings are just bad
bool exists___(const char* path) {
    struct stat buffer;
    return (stat (path, &buffer) == 0);
}

char* newFile(char* path, char* extension) {
    int len = strlen(path);
    static char cat[PATH_MAX+1];
    snprintf(cat, sizeof(cat), "%s.%s",
             path, extension);
    if (!exists___(cat)) {
        return cat;
    }
    int i = 1;
    do {
        snprintf(cat+len, sizeof(cat)-len, "%d.%s",
                 i, extension);

        i++;
    } while (exists___(cat));

    return cat;
}
