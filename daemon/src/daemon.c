#include <poll.h>
#include <stdint.h>
#include <stdlib.h>

#include "bulk.h"
#include "daemon.h"
#include "log.h"
#include "pulseaudio.h"

Context daemon_ctx;

static void libusb_io_cb(pa_mainloop_api *api, pa_io_event *e, int fd,
                         pa_io_event_flags_t events, void *userdata) {
  (void)api;
  (void)e;
  (void)fd;
  (void)events;
  struct timeval tv = {0, 0};
  libusb_handle_events_timeout((libusb_context *)userdata, &tv);
}

static int register_libusb_pollfds_with_pa(pa_mainloop_api *api,
                                           libusb_context *ctx) {
  const struct libusb_pollfd **poll_fds = libusb_get_pollfds(ctx);
  for (size_t i = 0; poll_fds[i]; ++i) {
    LOGI("adding poll_fd[%zu]", i);
    pa_io_event_flags_t flags = 0;
    if (poll_fds[i]->events & POLLIN) {
      LOGI("poll_fd[%zu] = pollin", i);
      flags |= PA_IO_EVENT_INPUT;
    }

    if (poll_fds[i]->events & POLLOUT) {
      flags |= PA_IO_EVENT_OUTPUT;
    }

    api->io_new(api, poll_fds[i]->fd, flags, libusb_io_cb, ctx);
  }
  return 0;
}

void LIBUSB_CALL transfer_cb(struct libusb_transfer *transfer) {
  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    printf("Got report (%d bytes): ", transfer->actual_length);
    for (int i = 0; i < transfer->actual_length; ++i)
      printf("%02x ", transfer->buffer[i]);
    printf("\n");

    libusb_submit_transfer(transfer);
  } else {
    LOGE("transfer error: %d", transfer->status);
    libusb_free_transfer(transfer);
    pa_mainloop_quit(daemon_ctx.mainloop, DAEMON_RETURN_NORETRY);
  }
}

int daemon_run(libusb_context *libusb_ctx) {
  int ret = 0;
  unsigned char buf[HID_REPORT_SIZE] = {0};

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

  struct libusb_transfer *transfer = libusb_alloc_transfer(0);
  libusb_fill_interrupt_transfer(transfer, daemon_ctx.handle, HID_IN_EPADDR,
                                 buf, HID_REPORT_SIZE, transfer_cb, NULL, 0);
  LOGI("submitting initial libusb transfer");
  ret = libusb_submit_transfer(transfer);
  if (ret < 0) {
    LOGE("error submitting initial libusb transfer [%d]", ret);
    libusb_free_transfer(transfer);
    ret = DAEMON_RETURN_NORETRY;
    goto out;
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
  register_libusb_pollfds_with_pa(mainloop_api, libusb_ctx);

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
