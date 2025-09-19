#ifndef DAEMON_H
#define DAEMON_H

enum DaemonReturnType {
  DAEMON_RETURN_RETRY,
  DAEMON_RETURN_NORETRY,
};

int daemon_run(void);
#endif // !DAEMON_H
