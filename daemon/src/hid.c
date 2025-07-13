#include "hid.h"
#include "daemon.h"
#include "log.h"
#include <string.h>

extern Context daemon_ctx;

int hid_write(uint8_t *report, size_t size) {
  int len, ret;
  ret = libusb_interrupt_transfer(daemon_ctx.handle, HID_EP_OUT, report, size,
                                  &len, 1000);
  if (ret < 0) {
    LOGE("error sending hid report - %s", libusb_error_name(ret));
    return ret;
  }

  if (len != (int)size) {
    LOGE("error sending hid report, sent %d bytes but expected %zu", len, size);
    return 1;
  }

  return 0;
}

static void transfer_cb(struct libusb_transfer *transfer) {
  uint8_t *buf = transfer->user_data;

  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    LOGE("HID transfer error: %d", transfer->status);
  } else if (transfer->actual_length != transfer->length) {
    LOGE("HID transfer incomplete: sent %d of %d bytes",
         transfer->actual_length, transfer->length);
  }

  /* clean up */
  free(buf);
  libusb_free_transfer(transfer);
}

/* enqueue an async interrupt transfer */
int hid_write_async(uint8_t *report, size_t size) {
  struct libusb_transfer *xfer;
  uint8_t *buf;
  int ret;

  /* allocate a transfer (0 = no isochronous packets) */
  xfer = libusb_alloc_transfer(0);
  if (!xfer) {
    LOGE("alloc_transfer failed");
    return -1;
  }

  /* we need our own copy because libusb will use it after this call */
  buf = malloc(size);
  if (!buf) {
    LOGE("malloc failed");
    libusb_free_transfer(xfer);
    return -1;
  }
  memcpy(buf, report, size);

  /* fill in the transfer */
  libusb_fill_interrupt_transfer(xfer, daemon_ctx.handle,
                                 HID_EP_OUT, /* your OUT endpoint address */
                                 buf, (int)size, transfer_cb,
                                 buf, /* user_data: so we can free(buf) in cb */
                                 1000 /* timeout in ms */
  );

  /* submit it; this returns immediately */
  ret = libusb_submit_transfer(xfer);
  if (ret < 0) {
    LOGE("submit_transfer error: %s", libusb_error_name(ret));
    free(buf);
    libusb_free_transfer(xfer);
    return ret;
  }

  return 0; /* queued successfully */
}
