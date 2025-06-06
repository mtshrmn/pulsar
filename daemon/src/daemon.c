#include <stdint.h>
#include <stdlib.h>

#include "bulk.h"
#include "daemon.h"
#include "log.h"
#include "pulseaudio.h"

Context daemon_ctx;

static int daemon_hid_write(libusb_device_handle *handle, uint8_t *report,
                            size_t size) {
  int len, ret;
  ret = libusb_interrupt_transfer(handle, HID_EP_OUT, report, size, &len, 1000);
  if (ret < 0) {
    return ret;
  }

  return len;
}

int daemon_run(void) {
  int ret = 0;
  uint8_t hid_report[HID_REPORT_SIZE] = {0};

  LOGI("starting daemon");

  daemon_ctx.handle = libusb_open_device_with_vid_pid(NULL, VID, PID);
  if (daemon_ctx.handle == NULL) {
    LOGE("failed opening HID device");
    return 0;
  }

  if (libusb_kernel_driver_active(daemon_ctx.handle, INTERFACE) == 1) {
    libusb_detach_kernel_driver(daemon_ctx.handle, INTERFACE);
  }

  ret = libusb_claim_interface(daemon_ctx.handle, INTERFACE);
  if (ret != 0) {
    LOGE("libusb failed to claim interface");
    libusb_close(daemon_ctx.handle);
    return 0;
  }

  daemon_ctx.mainloop = pa_mainloop_new();
  if (daemon_ctx.mainloop == NULL) {
    LOGE("error setting up pulseaudio mainloop");
    goto out;
  }

  pa_mainloop_api *mainloop_api = pa_mainloop_get_api(daemon_ctx.mainloop);
  if (mainloop_api == NULL) {
    LOGE("error getting pulseaudio mainloop api");
    goto out;
  }

  daemon_ctx.pa_ctx = pa_context_new(mainloop_api, "Volume Mixer Daemon");
  pa_context_set_state_callback(daemon_ctx.pa_ctx, context_state_cb,
                                &daemon_ctx);
  pa_context_connect(daemon_ctx.pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

  LOGI("starting pulseaudio mainloop");
  pa_mainloop_run(daemon_ctx.mainloop, &ret);
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
  pa_context_disconnect(daemon_ctx.pa_ctx);
  pa_context_unref(daemon_ctx.pa_ctx);
  pa_mainloop_free(daemon_ctx.mainloop);
  libusb_release_interface(daemon_ctx.handle, INTERFACE);
  libusb_close(daemon_ctx.handle);
  LOGI("stopping daemon");
  return ret;
}
