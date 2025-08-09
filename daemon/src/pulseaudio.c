#include "pulseaudio.h"
#include "log.h"
#include <pulse/pulseaudio.h>

static void libusb_io_cb(pa_mainloop_api *api, pa_io_event *e, int fd,
                         pa_io_event_flags_t events, void *userdata) {

  (void)api;
  (void)e;
  (void)fd;
  (void)events;
  (void)userdata;
  struct timeval tv = {0, 0};
  libusb_handle_events_timeout(NULL, &tv);
}

static void register_libusb_pollfds_with_pa(pa_mainloop_api *api) {
  const struct libusb_pollfd **poll_fds = libusb_get_pollfds(NULL);
  for (size_t i = 0; poll_fds[i]; ++i) {
    LOGI("adding poll_fd[%zu]", i);
    pa_io_event_flags_t flags = 0;
    if (poll_fds[i]->events & POLL_IN) {
      flags |= PA_IO_EVENT_INPUT;
    }

    if (poll_fds[i]->events & POLL_OUT) {
      flags |= PA_IO_EVENT_OUTPUT;
    }

    api->io_new(api, poll_fds[i]->fd, flags, libusb_io_cb, NULL);
  }
}

enum DaemonReturnType setup_pulseaudio_mainloop(void) {
  int ret;
  pa_context *context = NULL;

  pa_mainloop *mainloop = pa_mainloop_new();
  if (mainloop == NULL) {
    LOGE("error setting up pulseaudio mainloop");
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  pa_mainloop_api *mainloop_api = pa_mainloop_get_api(mainloop);
  if (mainloop_api == NULL) {
    LOGE("error getting pulseaudio mainloop api");
    ret = DAEMON_RETURN_NORETRY;
    goto out;
  }

  context = pa_context_new(mainloop_api, "Volume Mixer Daemon");
  pa_context_set_state_callback(context, NULL, NULL); // TODO: add callback
  pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL);
  register_libusb_pollfds_with_pa(mainloop_api);

  LOGI("starting pulseaudio mainloop");
  pa_mainloop_run(mainloop, &ret);

out:
  pa_context_disconnect(context);
  pa_context_unref(context);
  pa_mainloop_free(mainloop);
  return ret;
}
