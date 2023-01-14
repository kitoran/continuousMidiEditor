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

#ifdef __cplusplus
extern "C"
#endif
void reaperSetPosition(double d);

#ifdef __cplusplus
extern "C"
#endif
void reaperOnCommand(u32 command);
#endif // ACTIONS_H
