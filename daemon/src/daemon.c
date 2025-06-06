#include <libusb-1.0/libusb.h>
#include <pulse/pulseaudio.h>
#include <stdint.h>
#include <stdlib.h>

#include "bulk.h"
#include "daemon.h"
#include "log.h"
#include "pulseaudio.h"

static pa_context *pa_ctx = NULL;

static int daemon_hid_write(libusb_device_handle *handle, uint8_t *report,
                            size_t size) {
  int len, ret;
  ret = libusb_interrupt_transfer(handle, HID_EP_OUT, report, size, &len, 1000);
  if (ret < 0) {
    return ret;
  }

  return len;
}

void daemon_run(void) {
  int ret;
  libusb_device_handle *handle = NULL;
  uint8_t hid_report[HID_REPORT_SIZE] = {0};

  LOGI("starting daemon");

  handle = libusb_open_device_with_vid_pid(NULL, VID, PID);
  if (handle == NULL) {
    LOGE("failed opening HID device");
    return;
  }

  if (libusb_kernel_driver_active(handle, INTERFACE) == 1) {
    libusb_detach_kernel_driver(handle, INTERFACE);
  }

  ret = libusb_claim_interface(handle, INTERFACE);
  if (ret != 0) {
    LOGE("libusb failed to claim interface");
    libusb_close(handle);
    return;
  }

  pa_mainloop *mainloop = pa_mainloop_new();
  if (mainloop == NULL) {
    LOGE("error setting up pulseaudio mainloop");
    goto out;
  }

  pa_mainloop_api *mainloop_api = pa_mainloop_get_api(mainloop);
  if (mainloop_api == NULL) {
    LOGE("error getting pulseaudio mainloop api");
    goto out;
  }

  pa_ctx = pa_context_new(mainloop_api, "Volume Mixer Daemon");
  pa_context_set_state_callback(pa_ctx, context_state_cb, NULL);
  pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

  LOGI("starting pulseaudio mainloop");
  pa_mainloop_run(mainloop, &ret);
  if (ret != 0) {
    LOGE("pulseaudio mainloop quit unexpectedly");
  }

  // ret = daemon_hid_write(handle, hid_report, sizeof(hid_report));
  // if (ret != sizeof(hid_report)) {
  //   LOGE("error sending HID report [%d] - %s", ret, libusb_error_name(ret));
  //   goto out;
  // }
  //
  // ret = bulk_send_image(handle, "img/car2.png", 15, 40);
  // if (ret != 0) {
  //   goto out;
  // }

out:
  pa_context_disconnect(pa_ctx);
  pa_context_unref(pa_ctx);
  pa_mainloop_free(mainloop);
  libusb_release_interface(handle, INTERFACE);
  libusb_close(handle);
  LOGI("stopping daemon");
  return;
}
