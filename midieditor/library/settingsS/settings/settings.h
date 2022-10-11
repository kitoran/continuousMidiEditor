#ifndef SETTINGS_H
#define SETTINGS_H

int loadInt(char* name, _Bool *success);
void saveInt(char* name, int value);

#define SAVE_INT(v) saveInt(#v, v);

#endif // SETTINGS_H
