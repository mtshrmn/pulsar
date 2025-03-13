#include <errno.h>
#include <linux/netlink.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "hidapi.h"
#include "log.h"

#define UEVENT_BUFFER_SIZE 256

int main(void) {
  int sock_fd;
  char buffer[UEVENT_BUFFER_SIZE];
  struct sockaddr_nl addr = {
      .nl_family = AF_NETLINK,
      .nl_pid = getpid(),
      .nl_groups = -1,
      .nl_pad = 0,
  };
  ssize_t len;
  int ret;

  sock_fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
  if (sock_fd < 0) {
    LOGE("couldn't create a socket, reason - %s", strerror(errno));
    return -1;
  }

  if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    LOGE("couldn't bind socket, reason - %s", strerror(errno));
    ret = -1;
    goto out;
  }

  for (;;) {
    len = recv(sock_fd, buffer, sizeof(buffer), 0);
    if (len < 0) {
      LOGE("error when reading uevent, reason - %s", strerror(errno));
      ret = len;
      goto out;
    }

    // attempt connection to device
    ret = connect_to_device(buffer, sizeof(buffer));
    if (ret < 0) {
      goto out;
    }
  }

out:
  close(sock_fd);
  return ret;
}
