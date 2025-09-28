#include "hid.h"
#include "common/protocol.h"
#include "log.h"
#include <libusb-1.0/libusb.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define HIGH(x) ((uint8_t)((x) >> 8))
#define LOW(x) ((uint8_t)((x)))

extern libusb_device_handle *device_handle;

static HIDQueue report_queue = {
    .read_head = 0,
    .write_head = 0,
    .is_ready_to_dequeue = true,
};

static void interrupt_transfer_complete_cb(struct libusb_transfer *transfer) {
  if (transfer->status != LIBUSB_TRANSFER_COMPLETED) {
    LOGE("HID transfer error :%d", transfer->status);
    goto out;
  }

  if (transfer->actual_length != transfer->length) {
    LOGE("HID transfer incomplete (sent %d/%d bytes)", transfer->actual_length,
         transfer->length);
    goto out;
  }

out:
  libusb_free_transfer(transfer);
}

int hid_write(uint8_t *report, size_t size) {
  struct libusb_transfer *transfer;
  int ret;

  transfer = libusb_alloc_transfer(0);
  if (transfer == NULL) {
    LOGE("failed allocating libusb transfer");
    return -1;
  }

  libusb_fill_interrupt_transfer(transfer, device_handle, HID_OUT_EPADDR,
                                 report, size, interrupt_transfer_complete_cb,
                                 NULL, 1000);
  ret = libusb_submit_transfer(transfer);
  if (ret < 0) {
    LOGE("error submitting libusb transfer %s", libusb_error_name(ret));
    libusb_free_transfer(transfer);
    return ret;
  }

  return 0;
}

static RGB565 rgb888_to_rgb565(RGB888 *color) {
  RGB565 rgb565 = {
      .low = ((color->r >> 3) << 3) | color->g >> 5,
      .high = (color->g & 0xE0) | color->b >> 3,
  };
  return rgb565;
}

int hid_enqueue_report(uint8_t *report, size_t size) {
  if (size > HID_EPSIZE) {
    LOGE("report size (%zu) too big", size);
    return 1;
  }

  memcpy(report_queue.ring_buffer[report_queue.write_head], report, size);
  report_queue.write_head = (report_queue.write_head + 1) % RING_BUFFER_SIZE;

  // attempt dequeuing this report
  return hid_dequeue_report();
}

int hid_dequeue_report(void) {
  if (!report_queue.is_ready_to_dequeue) {
    LOGI("can't submit report, buffering instead");
    return 0;
  }

  if (report_queue.read_head == report_queue.write_head) {
    return 0;
  }

  uint8_t *report = report_queue.ring_buffer[report_queue.read_head];
  report_queue.read_head = (report_queue.read_head + 1) % RING_BUFFER_SIZE;
  report_queue.is_ready_to_dequeue = false;
  return hid_write(report, HID_EPSIZE);
}

void hid_report_queue_mark_ready(void) {
  report_queue.is_ready_to_dequeue = true;
}

int bulk_send_image(uint8_t index, const char *image_path, uint16_t x,
                    uint8_t y) {

  int ret;
  int len;

  int width;
  int height;
  int channels;

  uint8_t *data = stbi_load(image_path, &width, &height, &channels, 3);
  if (data == NULL) {
    LOGE("error loading image '%s', reason - %s", image_path,
         stbi_failure_reason());
    return 0;
  }
  LOGI("loaded image %s (w*h=%d*%d, c=%d)", image_path, width, height,
       channels);

  ImageData header = {
      .index = index,
      .x0 = x,
      .x1 = x + width - 1,
      .y0 = y,
      .y1 = y + height - 1,
      .data_len = 2 * width * height,
  };

  uint8_t *rgb565_data = malloc(header.data_len);
  if (rgb565_data == NULL) {
    LOGE("failed to allocate rgb565_data of size %d", header.data_len);
    ret = 1;
    goto out;
  }

  LOGI("converting RGB888 data to RGB565");
  for (int i = 0; i < width * height; ++i) {
    RGB565 rgb565_pixel = rgb888_to_rgb565((RGB888 *)(data + i * 3));
    *(RGB565 *)(rgb565_data + i * 2) = rgb565_pixel;
  }

  LOGI("sending header over bulk");
  ret = libusb_bulk_transfer(device_handle, BULK_OUT_EPADDR, (uint8_t *)&header,
                             sizeof(header), &len, 1000);
  if (ret != 0) {
    LOGE("bulk transfer failed - %s", libusb_error_name(ret));
    goto out;
  }

  if (len != sizeof(header)) {
    LOGE("bulk transfer incomplete (sent %d/%lu bytes)", len, sizeof(header));
    ret = 1;
    goto out;
  }

  LOGI("sending image data over bulk");
  ret = libusb_bulk_transfer(device_handle, BULK_OUT_EPADDR, rgb565_data,
                             header.data_len, &len, 1000);
  if (ret != 0) {
    LOGE("bulk transfer failed - %s", libusb_error_name(ret));
    goto out;
  }

  if (len != (int)header.data_len) {
    LOGE("bulk transfer incomplete (sent %d/%u bytes)", len, header.data_len);
    ret = 1;
    goto out;
  }

out:
  free(data);
  free(rgb565_data);
  return ret;
}
