#ifndef ACTIONS_H
#define ACTIONS_H
#include "melody.h"
#ifdef __cplusplus
extern "C" {
#endif
void play();
void togglePause();
void stop();
void toggleRepeat();

void reaperSetPosition(double d);

void reaperOnCommand(u32 command);
void startPlayingNote(double freq, int vel);
void stopPlayingNoteIfPlaying();
void MessageBoxInfo(char*title, char *message);
void message(const char* format, ...);

void reaperCommitChanges(char* undoName);
void reload();

void undo();
void redo();

void save();
#ifdef __cplusplus
}
#endif
#endif // ACTIONS_H
