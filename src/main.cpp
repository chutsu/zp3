#include "zp3.hpp"

int main(int argc, char **argv) {
  zp3_t zp3;
  if (zp3_init(zp3, "/data/music") != 0) {
    FATAL("Failed to initialize ZP3!");
  }
  zp3_loop(zp3);

  return 0;
}
