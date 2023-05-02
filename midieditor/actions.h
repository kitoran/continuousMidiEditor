#ifndef ACTIONS_H
#define ACTIONS_H
#include "melody.h"
#ifdef __cplusplus
extern "C" {
#endif
void play();
void stop();

void reaperInsert(RealNote note);
void reaperDeleteSelected();

void reaperSetPosition(double d);

void reaperOnCommand(u32 command);
void startPlayingNote(double freq);
void stopPlayingNote();
void MessageBoxInfo(char*title, char *message);
void message(const char* format, ...);

void reaperMoveNotes(double time, double freq);
void reaperCopyNotes();
void reload();

void undo();
void redo();

void save();
#ifdef __cplusplus
}
#endif
#endif // ACTIONS_H
