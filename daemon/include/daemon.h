#ifndef DAEMON_H
#define DAEMON_H

#include <libusb-1.0/libusb.h>
#include <pulse/pulseaudio.h>

#define VID 0x1209
#define PID 0x2711

#define ENDPOINT_DIR_IN 0x80

#define HID_REPORT_SIZE 8
#define INTERFACE 0
#define HID_EP_OUT 0x02
#define HID_IN_EPADDR (ENDPOINT_DIR_IN | 1)

typedef struct {
  pa_mainloop *mainloop;
  libusb_device_handle *handle;
  pa_context *pa_ctx;
} Context;

enum DaemonReturnType {
  DAEMON_RETURN_RETRY,
  DAEMON_RETURN_NORETRY,
};

int daemon_run(libusb_context *ctx);

#endif // !DAEMON_H
