#ifndef SINKCTL_H
#define SINKCTL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SINKS_NUM 1

typedef struct {
  char *name;
  double volume;
  bool is_stale;
} SinkInput;

int sink_input_info_cleanup(void);
int update_sinks(char *name, double volume);

#endif // !SINKCTL_H
