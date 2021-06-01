#include <fstream>
#include <iostream>

#include <fcntl.h>  /* For O_RDWR */
#include <unistd.h> /* For open(), creat() */
//#include io.h

class CpuMonitor {
public:
  CpuMonitor() : last_total_(0), last_idle_(0) {}

  virtual ~CpuMonitor() {}

  double Get() {
    char buf[4096];
    double total = 0;
    double idle = 0;
    double num = 0;
    char *buf_ = buf + 3;
    int fd = open("/proc/stat", O_RDONLY);
    int len = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    buf_[len] = '\0';
    total = 0;
    for (int i = 0; i < 20; i++) {
      num = strtod(buf_, &buf_);
      if (num == 0)
        break;
      total += num;
      if (i == 3)
        idle = num;
    }
    double percent = 100 * (1 - (idle - last_idle_) / (total - last_total_));
    last_total_ = total;
    last_idle_ = idle;
    return percent;
  }

  double last_total_;
  double last_idle_;
};