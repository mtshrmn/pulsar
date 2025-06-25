#include "pulseaudio.h"
#include "bulk.h"
#include "daemon.h"
#include "log.h"
#include "sinkctl.h"
#include <string.h>

extern Context daemon_ctx;

static void sink_input_info_cb(pa_context *ctx, const pa_sink_input_info *info,
                               int eol, void *data) {
  (void)ctx;
  (void)data;
  int ret = 0;

  if (eol || info == NULL) {
    ret = sink_input_info_cleanup();
    if (ret != 0) {
      pa_mainloop_quit(daemon_ctx.mainloop, DAEMON_RETURN_NORETRY);
    }
    return;
  }

  const char *app_name =
      pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
  // const char *icon_name =
  // pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_ICON_NAME);
  char *app_name_cpy = strdup(app_name);
  pa_volume_t volume = pa_cvolume_avg(&info->volume);
  double volume_percent = (volume * 100.f) / PA_VOLUME_NORM;
  LOGI("%s - %f%%", app_name_cpy, volume_percent);

  ret = update_sinks(app_name_cpy, volume_percent);
  if (ret != 0) {
    pa_mainloop_quit(daemon_ctx.mainloop, DAEMON_RETURN_NORETRY);
  }
}

static void get_sink_input_info_list(pa_context *ctx) {
  LOGI("dispatching operation get_sink_input_info_list");
  pa_operation *op;
  op = pa_context_get_sink_input_info_list(ctx, sink_input_info_cb, NULL);
  pa_operation_unref(op);
}

static void sink_input_event_cb(pa_context *ctx, pa_subscription_event_type_t t,
                                uint32_t idx, void *data) {
  (void)t;
  (void)idx;
  (void)data;
  get_sink_input_info_list(ctx);
}

void context_state_cb(pa_context *ctx, void *data) {
  (void)data;
  if (pa_context_get_state(ctx) != PA_CONTEXT_READY) {
    return;
  }

  LOGI("get list of sink inputs initially");
  get_sink_input_info_list(ctx);

  LOGI("subscribing to pulseaudio events");
  pa_context_set_subscribe_callback(ctx, sink_input_event_cb, NULL);
  pa_context_subscribe(ctx, PA_SUBSCRIPTION_MASK_SINK_INPUT, NULL, NULL);
}
