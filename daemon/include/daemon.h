#ifndef DAEMON_H
#define DAEMON_H

#define VID 0x1209
#define PID 0x2711

#define HID_REPORT_SIZE 8
#define INTERFACE 0
#define HID_EP_OUT 0x02
#define BULK_EP_OUT 0x03

void daemon_run(void);

#endif // !DAEMON_H
