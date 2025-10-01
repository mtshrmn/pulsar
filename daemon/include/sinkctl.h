#ifndef SINKCTL_H
#define SINKCTL_H

#include <pulse/pulseaudio.h>

void sinkctl_init_displays(void);
int sinkctl_insert_sink(const pa_sink_input_info *info);
int sinkctl_update_sink(const pa_sink_input_info *info);
int sinkctl_remove_sink(int idx);
int sinkctl_volume_inc(int);
int sinkctl_volume_dec(int);
#endif
