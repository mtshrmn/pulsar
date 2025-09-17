#include "sinkctl.h"
#include "log.h"

int sinkctl_insert_sink(UNUSED const pa_sink_input_info *info) {
  const char *name = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
  LOGI("insert sink was called (%s)", name);
  return 0;
}

int sinkctl_update_sink(UNUSED const pa_sink_input_info *info) {
  const char *name = pa_proplist_gets(info->proplist, PA_PROP_APPLICATION_NAME);
  LOGI("update sink was called (%s)", name);
  return 0;
}
int sinkctl_remove_sink(UNUSED int idx) {
  LOGI("remove sink was called");
  return 0;
}
