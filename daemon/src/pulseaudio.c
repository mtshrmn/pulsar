#include "pulseaudio.h"
#include "log.h"

static void sink_input_info_cb(pa_context *ctx, const pa_sink_input_info *info,
                               int eol, void *data) {
  (void)data;
  if (eol || info == NULL) {
    return;
  }

  const char *app_name = pa_proplist_gets(info->proplist, "application.name");
  LOGI("found pulseaudio sink input - \"%s\"", app_name);
}

static void get_sink_input_info_list(pa_context *c) {
  LOGI("displatching operation get_sink_input_info_list");
  pa_operation *op;
  op = pa_context_get_sink_input_info_list(c, sink_input_info_cb, NULL);
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

  // get list of sink inputs initially.
  get_sink_input_info_list(ctx);

  LOGI("subscribing to pulseaudio events");
  pa_context_set_subscribe_callback(ctx, sink_input_event_cb, NULL);
  pa_context_subscribe(ctx, PA_SUBSCRIPTION_MASK_SINK_INPUT, NULL, NULL);
}
