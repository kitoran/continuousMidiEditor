#ifndef ACTIONS_H
#define ACTIONS_H
#ifdef __cplusplus
extern "C"
#endif
void play();
#ifdef __cplusplus
extern "C"
#endif
void stop();

#ifdef __cplusplus
extern "C"
#endif
void reaperInsert(Note note);
#ifdef __cplusplus
extern "C"
#endif
void reaperDelete(int note);

#endif // ACTIONS_H
