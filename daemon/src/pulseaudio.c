#include "pulseaudio.h"
#include "log.h"
#include "sinkctl.h"
#include <poll.h>
#include <pulse/pulseaudio.h>

static void libusb_io_cb(UNUSED pa_mainloop_api *api, UNUSED pa_io_event *e,
                         UNUSED int fd, UNUSED pa_io_event_flags_t events,
                         UNUSED void *userdata) {

  struct timeval tv = {0, 0};
  libusb_handle_events_timeout(NULL, &tv);
}

static void register_libusb_pollfds_with_pa(pa_mainloop_api *api) {
  const struct libusb_pollfd **poll_fds = libusb_get_pollfds(NULL);
  for (size_t i = 0; poll_fds[i]; ++i) {
    LOGI("adding poll_fd[%zu]", i);
    pa_io_event_flags_t flags = 0;
    if (poll_fds[i]->events & POLLIN) {
      flags |= PA_IO_EVENT_INPUT;
    }

    if (poll_fds[i]->events & POLLOUT) {
      flags |= PA_IO_EVENT_OUTPUT;
    }

    api->io_new(api, poll_fds[i]->fd, flags, libusb_io_cb, NULL);
  }
}

static void sink_input_new_cb(UNUSED pa_context *ctx,
                              const pa_sink_input_info *info, int eol,
                              void *data) {
  if (eol || info == NULL) {
    return;
  }

  int ret = sinkctl_insert_sink(info);
  if (ret != 0) {
    pa_mainloop_quit(data, DAEMON_RETURN_NORETRY);
  }
}

static void sink_input_change_cb(UNUSED pa_context *ctx,
                                 const pa_sink_input_info *info, int eol,
                                 void *data) {
  if (eol || info == NULL) {
    return;
  }

  int ret = sinkctl_update_sink(info);
  if (ret != 0) {
    pa_mainloop_quit(data, DAEMON_RETURN_NORETRY);
  }
}

static void sink_input_info_list_cb(UNUSED pa_context *ctx,
                                    const pa_sink_input_info *info, int eol,
                                    void *data) {
  if (eol || info == NULL) {
    return;
  }
  // TODO:
  LOGI("sink input info list was called");
}

static void sink_input_event_cb(pa_context *ctx, pa_subscription_event_type_t t,
                                uint32_t idx, void *data) {
  pa_operation *op;
  pa_subscription_event_type_t event = t & PA_SUBSCRIPTION_EVENT_TYPE_MASK;
  // pa_mainloop *mainloop = (pa_mainloop *)data;
  switch (event) {
  case PA_SUBSCRIPTION_EVENT_NEW:
    op = pa_context_get_sink_input_info(ctx, idx, sink_input_new_cb, data);
    pa_operation_unref(op);
    break;
  case PA_SUBSCRIPTION_EVENT_CHANGE:
    op = pa_context_get_sink_input_info(ctx, idx, sink_input_change_cb, data);
    pa_operation_unref(op);
    break;
  case PA_SUBSCRIPTION_EVENT_REMOVE: {
    int ret = sinkctl_remove_sink(idx);
    if (ret != 0) {
      pa_mainloop_quit(data, DAEMON_RETURN_NORETRY);
    }
  } break;
  default:
    break;
  }
}

static void context_state_cb(pa_context *ctx, void *data) {
  if (pa_context_get_state(ctx) != PA_CONTEXT_READY) {
    return;
  }

  LOGI("get initial list of sink inputs");
  // pa_mainloop *mainloop = (pa_mainloop *)data;
  pa_operation *op;
  op = pa_context_get_sink_input_info_list(ctx, sink_input_info_list_cb, data);
  pa_operation_unref(op);

  LOGI("subscribing to pulseaudio events");
  pa_context_set_subscribe_callback(ctx, sink_input_event_cb, data);
  pa_context_subscribe(ctx, PA_SUBSCRIPTION_MASK_SINK_INPUT, NULL, NULL);
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
  pa_context_set_state_callback(context, context_state_cb, mainloop);
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
