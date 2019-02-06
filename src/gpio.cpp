#include "gpio.hpp"

int gpio_enable(const int pin) {
  int fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio export for writing!\n");
    return -1;
  }

  char buffer[GPIO_BUFFER_MAX];
  ssize_t bytes_written = snprintf(buffer, GPIO_BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);

  return 0;
}

int gpio_disable(const int pin) {
  int fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio unexport for writing!\n");
    return -1;
  }

  char buffer[GPIO_BUFFER_MAX];
  ssize_t bytes_written;
  bytes_written = snprintf(buffer, GPIO_BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);

  return 0;
}

int gpio_direction(const int pin, const int dir) {
  char path[GPIO_DIRECTION_MAX];
  snprintf(path, GPIO_DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  int fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio direction for writing!\n");
    return -1;
  }

  static const char s_directions_str[]  = "in\0out";
  if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
    fprintf(stderr, "Failed to set direction!\n");
    return -1;
  }

  close(fd);
  return 0;
}

int gpio_read(const int pin) {
  char path[GPIO_VALUE_MAX];
  snprintf(path, GPIO_VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  int fd = open(path, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for reading!\n");
    return -1;
  }

  char value_str[3];
  if (-1 == read(fd, value_str, 3)) {
    fprintf(stderr, "Failed to read value!\n");
    return -1;
  }

  close(fd);
  return atoi(value_str);
}

int gpio_write(const int pin, const int value) {
  char path[GPIO_VALUE_MAX];
  snprintf(path, GPIO_VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  int fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for writing!\n");
    return -1;
  }

  static const char s_values_str[] = "01";
  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
    fprintf(stderr, "Failed to write value!\n");
    return -1;
  }

  close(fd);
  return 0;
}
