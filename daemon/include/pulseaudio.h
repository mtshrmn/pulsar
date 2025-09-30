#ifndef PULSEAUDIO_H
#define PULSEAUDIO_H

#include "daemon.h"
#include <pulse/context.h>
#include <pulse/mainloop.h>

void setup_pulseaudio_mainloop(pa_mainloop *mainloop);
pa_context *pulseaudio_get_pa_context(void);
#endif // !PULSEAUDIO_H
