#include <libusb-1.0/libusb.h>
#include <pthread.h>
#include <stdio.h>

#include "daemon.h"
#include "log.h"

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static int LIBUSB_CALL hp_callback(struct libusb_context *ctx,
                                   struct libusb_device *dev,
                                   libusb_hotplug_event event,
                                   void *user_data) {
  struct libusb_device_descriptor desc;
  (void)ctx;
  (void)user_data;
  int ret;

  if (event != LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) {
    return 0;
  }

  ret = libusb_get_device_descriptor(dev, &desc);
  if (ret != 0) {
    return 0;
  }

  if (desc.idVendor != VID || desc.idProduct != PID) {
    return 0;
  }

  // tell daemon thread to run again.
  pthread_mutex_lock(&mtx);
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mtx);

  return 0;
}

void *daemon_worker(void *arg) {
  (void)arg;
  // signal to master that daemon is ready.
  pthread_mutex_lock(&mtx);
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mtx);

  for (;;) {
    pthread_mutex_lock(&mtx);
    pthread_cond_wait(&cond, &mtx);
    pthread_mutex_unlock(&mtx);

    daemon_run();
  }

  return NULL;
}

int main(void) {
  libusb_context *ctx = NULL;
  libusb_hotplug_callback_handle hp_handle;
  int ret;
  pthread_t daemon_thread;

  ret = libusb_init(&ctx);
  if (ret != 0) {
    LOGE("failed to initialize libusb - %s", libusb_error_name(ret));
    return 1;
  }

  pthread_create(&daemon_thread, NULL, daemon_worker, ctx);

  // wait for daemon thread to start
  pthread_mutex_lock(&mtx);
  pthread_cond_wait(&cond, &mtx);

  // try running the daemon already
  pthread_cond_signal(&cond);
  pthread_mutex_unlock(&mtx);

  if (!libusb_has_capability(LIBUSB_CAP_HAS_HOTPLUG)) {
    LOGE("hotplug isn't supported");
    libusb_exit(ctx);
    return 1;
  }

  ret = libusb_hotplug_register_callback(
      ctx, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED, 0, VID, PID,
      LIBUSB_HOTPLUG_MATCH_ANY, hp_callback, NULL, &hp_handle);
  if (ret != LIBUSB_SUCCESS) {
    LOGE("error registering hotplug callback - %s", libusb_error_name(ret));
    libusb_exit(ctx);
    return 1;
  }

  LOGI("hotplug succeful, listening to libusb events");

  for (;;) {
    ret = libusb_handle_events_completed(ctx, NULL);
    if (ret != LIBUSB_SUCCESS) {
      LOGE("libusb_handle_events() - %s", libusb_error_name(ret));
      break;
    }
  }

  libusb_hotplug_deregister_callback(ctx, hp_handle);
  pthread_join(daemon_thread, NULL);
  libusb_exit(ctx);
  return 0;
}
