#ifndef PULSEAUDIO_H
#define PULSEAUDIO_H

#include "daemon.h"
#include <pulse/mainloop.h>

void setup_pulseaudio_mainloop(pa_mainloop *mainloop);
#endif // !PULSEAUDIO_H
