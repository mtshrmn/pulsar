#include <stdint.h>
#include <stdlib.h>

#include "bulk.h"
#include "daemon.h"
#include "log.h"
#include "pulseaudio.h"

Context daemon_ctx;

int daemon_run(void) {
  int ret = 0;

  LOGI("starting daemon");

  daemon_ctx.handle = libusb_open_device_with_vid_pid(NULL, VID, PID);
  if (daemon_ctx.handle == NULL) {
    LOGE("failed opening HID device");
    return DAEMON_RETURN_NORETRY;
  }

  if (libusb_kernel_driver_active(daemon_ctx.handle, INTERFACE) == 1) {
    libusb_detach_kernel_driver(daemon_ctx.handle, INTERFACE);
  }

  ret = libusb_claim_interface(daemon_ctx.handle, INTERFACE);
  if (ret != 0) {
    LOGE("libusb failed to claim interface");
    libusb_close(daemon_ctx.handle);
    return DAEMON_RETURN_NORETRY;
  }

  daemon_ctx.mainloop = pa_mainloop_new();
  if (daemon_ctx.mainloop == NULL) {
    LOGE("error setting up pulseaudio mainloop");
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  pa_mainloop_api *mainloop_api = pa_mainloop_get_api(daemon_ctx.mainloop);
  if (mainloop_api == NULL) {
    LOGE("error getting pulseaudio mainloop api");
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  daemon_ctx.pa_ctx = pa_context_new(mainloop_api, "Volume Mixer Daemon");
  pa_context_set_state_callback(daemon_ctx.pa_ctx, context_state_cb,
                                &daemon_ctx);
  pa_context_connect(daemon_ctx.pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL);

  LOGI("starting pulseaudio mainloop");
  pa_mainloop_run(daemon_ctx.mainloop, &ret);
  if (ret == DAEMON_RETURN_RETRY) {
    LOGE("pulseaudio mainloop quit unexpectedly, restarting daemon");
  }

out:
  pa_context_disconnect(daemon_ctx.pa_ctx);
  pa_context_unref(daemon_ctx.pa_ctx);
  pa_mainloop_free(daemon_ctx.mainloop);
  libusb_release_interface(daemon_ctx.handle, INTERFACE);
  libusb_close(daemon_ctx.handle);
  LOGI("stopping daemon");
  return ret;
}
