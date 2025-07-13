#include "sinkctl.h"
#include "bulk.h"
#include "daemon.h"
#include "hid.h"
#include "log.h"
#include <string.h>

extern Context daemon_ctx;
static SinkInput *sinks[SINKS_NUM] = {0};

static void sinkinput_free(SinkInput *si) {
  free(si->name);
  free(si);
}

static SinkInput *sinkinput_create(char *name, double volume) {
  SinkInput *si = malloc(sizeof(SinkInput));
  si->name = name;
  si->volume = volume;
  si->is_stale = false;
  return si;
}

static int clear_display(uint8_t index) {
  uint8_t report[2] = {index, 1};
  return hid_write_async(report, sizeof(report));
}

int update_sinks(char *name, double volume) {
  // TODO: replace with actual logo fetching.
  char *image = "img/car2.png";
  if (strcmp(name, "Spotify") == 0) {
    image = "img/hat.png";
  }

  int ret = 0;
  // if sink already registered, do nothing.
  LOGI("updating sinks with \"%s\"", name);
  for (size_t i = 0; i < SINKS_NUM; ++i) {
    if (sinks[i] == NULL) {
      break;
    }
    if (strcmp(sinks[i]->name, name) == 0) {
      sinks[i]->is_stale = false;
      return 0;
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
    clear_display(i);
    ret = bulk_send_image(daemon_ctx.handle, i, image, 20, 20);
    if (ret != 0) {
      return ret;
    }
  }

  sinks[0] = sinkinput_create(name, volume);
  clear_display(0);
  ret = bulk_send_image(daemon_ctx.handle, 0, image, 20, 20);
  return ret;
}

static void mark_all_sinks_as_stale(void) {
  for (size_t i = 0; i < SINKS_NUM; ++i) {
    if (sinks[i] != NULL) {
      sinks[i]->is_stale = true;
    }
  }
}

static int remove_stale_sinks(void) {
  int ret = 0;
  for (size_t i = 0; i < SINKS_NUM; ++i) {
    if (sinks[i] == NULL) {
      continue;
    }

    if (sinks[i]->is_stale) {
      LOGI("removing stale sink \"%s\"", sinks[i]->name);
      sinkinput_free(sinks[i]);
      sinks[i] = NULL;
      ret = clear_display(i);
      if (ret != 0) {
        return ret;
      }
    }
  }
  mark_all_sinks_as_stale();
  return 0;
}

int sink_input_info_cleanup(void) {
  // for now, nothing else to do
  return remove_stale_sinks();
}
