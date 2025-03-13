#include "hidapi.h"
#include "log.h"

#include <hidapi/hidapi.h>

int connect_to_device(const char *buf, size_t size) {
  (void)buf;
  (void)size;

  hid_device *handle = hid_open(VID, PID, NULL);
  if (handle == NULL) {
    LOGI("failed opening device");
    return 0;
  }

  return 0;
}
