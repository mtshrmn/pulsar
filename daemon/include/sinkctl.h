#ifndef SINKCTL_H
#define SINKCTL_H

#include <pulse/pulseaudio.h>

#define INVALID_SINK_INDEX -1

typedef struct {
  int index;
  const char *name;
  int volume_percent;
} SinkInfo;

SinkInfo get_sink_info(const pa_sink_input_info *info);
void sinkctl_init_displays(void);
int sinkctl_insert_sink(const pa_sink_input_info *info);
int sinkctl_update_sink(const pa_sink_input_info *info);
int sinkctl_remove_sink(int idx);
#endif
