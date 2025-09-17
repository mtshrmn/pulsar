#ifndef DAEMON_H
#define DAEMON_H

#include <libusb-1.0/libusb.h>

enum DaemonReturnType {
  DAEMON_RETURN_RETRY,
  DAEMON_RETURN_NORETRY,
};

int daemon_run(libusb_device_handle *handle);
#endif // !DAEMON_H
