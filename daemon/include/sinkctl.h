#ifndef SINKCTL_H
#define SINKCTL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SINKS_NUM 1

typedef struct {
  char *name;
  bool is_stale;
} SinkInput;

int sink_input_info_cleanup(void);
int update_sinks(char *name);

#endif // !SINKCTL_H
