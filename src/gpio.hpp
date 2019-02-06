#ifndef ZP3_GPIO_HPP
#define ZP3_GPIO_HPP

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define GPIO_BUFFER_MAX 3
#define GPIO_VALUE_MAX 30
#define GPIO_DIRECTION_MAX 35

#define IN  0
#define OUT 1

#define LOW  0
#define HIGH 1

#define PIN  24 /* P1-18 */
#define POUT 4  /* P1-07 */

int gpio_enable(const int pin);
int gpio_disable(const int pin);
int gpio_direction(const int pin, const int dir);
int gpio_read(const int pin);
int gpio_write(const int pin, const int value);

#endif // ZP3_GPIO_HPP
