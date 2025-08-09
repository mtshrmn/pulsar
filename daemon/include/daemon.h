#ifndef DAEMON_H
#define DAEMON_H

#include <libusb-1.0/libusb.h>

#define VID 0x1209
#define PID 0x2711

#define ENDPOINT_DIR_IN 0x80
#define ENDPOINT_DIR_OUT 0

#define HID_REPORT_SIZE 8
#define INTERFACE 0
#define HID_IN_EPADDR (ENDPOINT_DIR_IN | 1)
#define HID_OUT_EPADDR (ENDPOINT_DIR_OUT | 2)

enum DaemonReturnType {
  DAEMON_RETURN_RETRY,
  DAEMON_RETURN_NORETRY,
};

int daemon_run(libusb_device_handle *handle);
#endif // !DAEMON_H
