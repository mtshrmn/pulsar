#ifndef PULSEAUDIO_H
#define PULSEAUDIO_H

#include <pulse/pulseaudio.h>
#include <stdbool.h>

void context_state_cb(pa_context *ctx, void *data);

#endif // !PULSEAUDIO_H
