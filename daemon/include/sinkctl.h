#ifndef SINKCTL_H
#define SINKCTL_H

#include <pulse/pulseaudio.h>
#include <stdbool.h>

typedef struct {
  const char *app_name;
  const char *icon_name;
  double volume_percent;
  int idx;
  bool stale;
} SinkInfo;

SinkInfo get_sink_info(pa_sink_info *info);
int sinkctl_insert_sink(const pa_sink_input_info *info);
int sinkctl_update_sink(const pa_sink_input_info *info);
int sinkctl_remove_sink(int idx);
#endif
