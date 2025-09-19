#include "common/protocol.h"
#include "daemon.h"
#include "log.h"
#include <libusb-1.0/libusb.h>
#include <stdbool.h>

libusb_device_handle *device_handle = NULL;

static void daemon_runner_wrapper(void) {
  int ret;

  if (device_handle == NULL) {
    return;
  }

  do {
    ret = daemon_run();
  } while (ret == DAEMON_RETURN_RETRY);
  libusb_close(device_handle);
  device_handle = NULL;
}

static int open_libusb_device(void) {
  int ret;
  libusb_device **devices;
  struct libusb_device_descriptor desc;

  ret = libusb_get_device_list(NULL, &devices);
  if (ret < 0) {
    LOGE("error getting libusb device list - %d", ret);
    return LIBUSB_ERROR_OTHER;
  }

  ret = LIBUSB_ERROR_OTHER;

  for (size_t i = 0; devices[i] != NULL; ++i) {
    if (libusb_get_device_descriptor(devices[i], &desc) != LIBUSB_SUCCESS) {
      continue;
    }

    if (desc.idVendor != VID || desc.idProduct != PID) {
      continue;
    }

    ret = libusb_open(devices[i], &device_handle);
    if (ret != LIBUSB_SUCCESS) {
      LOGE("error opening device - %s", libusb_error_name(ret));
      goto out;
    }
    LOGI("opened device!");
  }

out:
  libusb_free_device_list(devices, 0);
  return ret;
}

static int hotplug_callback(UNUSED struct libusb_context *ctx,
                            struct libusb_device *dev,
                            libusb_hotplug_event event,
                            UNUSED void *user_data) {
  struct libusb_device_descriptor desc;
  int ret;

  if (event != LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
    return 0;
  }

  ret = libusb_get_device_descriptor(dev, &desc);
  if (ret != LIBUSB_SUCCESS) {
    LOGE("failed getting device descriptor - %s", libusb_error_name(ret));
    return 0;
  }

  if (desc.idVendor != VID || desc.idProduct != PID) {
    return 0;
  }

  ret = libusb_open(dev, &device_handle);
  if (ret != LIBUSB_SUCCESS) {
    LOGE("failed opening usb device - %s", libusb_error_name(ret));
    return 0;
  }

  return 0;
}

int main(void) {
  int ret;

  ret = libusb_init_context(NULL, NULL, 0);
  if (ret != LIBUSB_SUCCESS) {
    LOGE("failed to initialize libusb - %s", libusb_error_name(ret));
    return 1;
  }

  // try running daemon
  ret = open_libusb_device();
  if (ret == LIBUSB_SUCCESS) {
    daemon_runner_wrapper();
  }

  if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
    LOGE("hotplug isn't supported");
    goto out;
  }

  ret = libusb_hotplug_register_callback(
      NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, VID, PID,
      LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL, NULL);

  LOGI("hotplug succeful, listening to libusb events");

  while (true) {
    ret = libusb_handle_events_completed(NULL, NULL);
    if (ret != 0) {
      LOGE("error handling events - %s", libusb_error_name(ret));
      continue;
    }

    daemon_runner_wrapper();
  }

out:
  libusb_release_interface(device_handle, INTERFACE_ID_HID);
  libusb_close(device_handle);
  libusb_exit(NULL);
  return 0;
}
