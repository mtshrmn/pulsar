#ifndef HIDAPI_H
#define HIDAPI_H

#include <stddef.h>

#define VID 0x1209
#define PID 0x2711

int connect_to_device(const char *buf, size_t size);

#endif
