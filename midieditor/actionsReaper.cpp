#include "actions.h"
#include "reaper_plugin_functions.h"
//TODO: need to call these from the main thread
void play() {
  OnPlayButton();
}

void stop() {
  OnStopButton();
}
