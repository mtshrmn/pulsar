#include "sinkctl.h"
#include "common/protocol.h"
#include "hid.h"
#include "log.h"
#include <string.h>
#include <unistd.h>

static SinkInfo displays[NUM_DISPLAYS];

static char *get_image_path_from_sink_info(const pa_sink_input_info *info) {
  const char *xdg_config = getenv("XDG_CONFIG_HOME");
  const char *name = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
  const char *subdir = "/mixer/";
  const char *ext = ".png";
  size_t len = strlen(xdg_config) + strlen(subdir) + strlen(name) + strlen(ext);
  char *image_path = malloc(len + 1); // plus one for null termination
  snprintf(image_path, len + 1, "%s%s%s%s", xdg_config, subdir, name, ext);
  return image_path;
}

void sinkctl_init_displays(void) {
  for (size_t i = 0; i < NUM_DISPLAYS; ++i) {
    displays[i].index = INVALID_SINK_INDEX;
    HIDReport report = {
        .index = i,
        .report_type = REPORT_TYPE_CLEAR,
    };

    hid_enqueue_report((uint8_t *)&report, sizeof(report));
  }
}

int sinkctl_insert_sink(const pa_sink_input_info *info) {
  SinkInfo sink_info = get_sink_info(info);
  LOGI("volume: %d", sink_info.volume_percent);
  for (size_t i = 0; i < NUM_DISPLAYS; ++i) {
    if (displays[i].index != INVALID_SINK_INDEX) {
      continue;
    }

    displays[i] = sink_info;
    LOGI("displays[%zu] = {.index = %d, ...}", i, displays[i].index);

    // TODO: change 0, 0 to center of screen inside bulk_send_image.
    char *image_path = get_image_path_from_sink_info(info);
    int ret = bulk_send_image(i, image_path, 20, 20);
    if (ret != 0) {
      LOGE("error sending image '%s'", image_path);
      free(image_path);
      return ret;
    }

    free(image_path);

    HIDReport report = {
        .index = i,
        .report_type = REPORT_TYPE_INIT,
    };
    ret = hid_enqueue_report((uint8_t *)&report, sizeof(report));
    if (ret != 0) {
      LOGE("error sending INIT report");
      return ret;
    }

    report = (HIDReport){
        .index = i,
        .report_type = REPORT_TYPE_SET_VOLUME,
        .volume = sink_info.volume_percent,
    };

    return hid_enqueue_report((uint8_t *)&report, sizeof(report));
  }

  return 0;
}

int sinkctl_update_sink(const pa_sink_input_info *info) {
  SinkInfo sink_info = get_sink_info(info);
  for (size_t i = 0; i < NUM_DISPLAYS; ++i) {
    if (displays[i].index != sink_info.index) {
      continue;
    }

    displays[i].volume_percent = sink_info.volume_percent;

    HIDReport report = {
        .index = i,
        .report_type = REPORT_TYPE_SET_VOLUME,
        .volume = sink_info.volume_percent,
    };

    return hid_enqueue_report((uint8_t *)&report, sizeof(report));
  }

  return 0;
}

int sinkctl_remove_sink(int index) {
  for (size_t i = 0; i < NUM_DISPLAYS; ++i) {
    if (displays[i].index != index) {
      continue;
    }

    displays[i].index = INVALID_SINK_INDEX;
    HIDReport report = {
        .index = i,
        .report_type = REPORT_TYPE_CLEAR,
    };

    return hid_enqueue_report((uint8_t *)&report, sizeof(report));
  }

  return 0;
}

SinkInfo get_sink_info(const pa_sink_input_info *info) {
  const char *name = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
  pa_volume_t volume = pa_cvolume_avg(&info->volume);
  int volume_percent = (volume * 100) / PA_VOLUME_NORM;
  return (SinkInfo){
      .index = info->index,
      .name = name,
      .volume_percent = volume_percent,
  };
}
