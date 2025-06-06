#include "pulseaudio.h"
#include "bulk.h"
#include "daemon.h"
#include "log.h"
#include <string.h>

#define SINKS_NUM 1
static SinkInput *sinks[SINKS_NUM] = {0};
extern Context daemon_ctx;

static void sinkinput_free(SinkInput *si) {
  free(si->name);
  free(si);
}

static SinkInput *sinkinput_create(char *name) {
  SinkInput *si = malloc(sizeof(SinkInput));
  si->name = name;
  si->is_stale = false;
  return si;
}

static void update_sinks(char *name) {
  // if sink already registered, do nothing.
  LOGI("updating sinks with \"%s\"", name);
  for (size_t i = 0; i < SINKS_NUM; ++i) {
    if (sinks[i] == NULL) {
      break;
    }
    if (strcmp(sinks[i]->name, name) == 0) {
      sinks[i]->is_stale = false;
      return;
    }
  }

  size_t free_slot = SINKS_NUM;
  for (size_t i = 0; i < SINKS_NUM; ++i) {
    if (sinks[i] == NULL) {
      free_slot = i;
      break;
    }
  }
  LOGI("free_slot=%zu", free_slot);

  // if sinks is full, discard last item
  // and set the free slot to point to last element.
  if (free_slot == SINKS_NUM) {
    sinkinput_free(sinks[SINKS_NUM - 1]);
    sinks[SINKS_NUM - 1] = NULL;
    free_slot = SINKS_NUM - 1;
  }

  // shift sinks to the right.
  for (size_t i = free_slot; i > 0; --i) {
    sinks[i] = sinks[i - 1];
  }

  sinks[0] = sinkinput_create(name);
}

static void mark_all_sinks_as_stale(void) {
  for (size_t i = 0; i < SINKS_NUM; ++i) {
    if (sinks[i] != NULL) {
      sinks[i]->is_stale = true;
    }
  }
}

static void remove_stale_sinks(void) {
  for (size_t i = 0; i < SINKS_NUM; ++i) {
    if (sinks[i] == NULL) {
      continue;
    }

    if (sinks[i]->is_stale) {
      LOGI("removing stale sink \"%s\"", sinks[i]->name);
      sinkinput_free(sinks[i]);
      sinks[i] = NULL;
    }
  }
  mark_all_sinks_as_stale();
}

static int sink_input_info_cleanup(void) {
  int ret = 0;
  remove_stale_sinks();
  for (size_t i = 0; i < SINKS_NUM; ++i) {
    if (sinks[i] == NULL) {
      continue;
    }

    ret = bulk_send_image(daemon_ctx.handle, "img/car2.png", 0, 0);
  }

  return ret;
}

static void sink_input_info_cb(pa_context *ctx, const pa_sink_input_info *info,
                               int eol, void *data) {
  (void)ctx;
  (void)data;
  int ret = 0;

  if (eol || info == NULL) {
    ret = sink_input_info_cleanup();
    if (ret != 0) {
      LOGE("fatal error occured, restarting daemon");
      pa_mainloop_quit(daemon_ctx.mainloop, 1);
    }
    return;
  }

  const char *app_name =
      pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
  const char *icon_name =
      pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_ICON_NAME);
  LOGI("icon_name = %s", icon_name);
  char *app_name_cpy = strdup(app_name);
  update_sinks(app_name_cpy);
}

static void get_sink_input_info_list(pa_context *ctx) {
  LOGI("displatching operation get_sink_input_info_list");
  pa_operation *op;
  mark_all_sinks_as_stale();
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
