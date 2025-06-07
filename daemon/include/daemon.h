#ifndef DAEMON_H
#define DAEMON_H

#include <libusb-1.0/libusb.h>
#include <pulse/pulseaudio.h>

#define VID 0x1209
#define PID 0x2711

#define HID_REPORT_SIZE 8
#define INTERFACE 0
#define HID_EP_OUT 0x02

typedef struct {
  pa_mainloop *mainloop;
  libusb_device_handle *handle;
  pa_context *pa_ctx;
} Context;

enum DaemonReturnType {
  DAEMON_RETURN_RETRY,
  DAEMON_RETURN_NORETRY,
};

int daemon_run(void);

#endif // !DAEMON_H
