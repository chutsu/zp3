#ifndef ZP3_TEST_HPP
#define ZP3_TEST_HPP

#define KNRM "\x1B[1;0m"
#define KRED "\x1B[1;31m"
#define KGRN "\x1B[1;32m"
#define KYEL "\x1B[1;33m"
#define KBLU "\x1B[1;34m"
#define KMAG "\x1B[1;35m"
#define KCYN "\x1B[1;36m"
#define KWHT "\x1B[1;37m"

#define CHECK(COND) \
  if (COND != true) { \
    return -1; \
  }

#define RUN_TEST(TEST) \
  printf("%sTEST%s [%s] ", KBLU, KNRM, #TEST); \
  fflush(stdout); \
  if (TEST() == 0) { \
    printf("%sPASSED%s\n", KGRN, KNRM); \
  } else { \
    printf("%sFAILED%s\n", KRED, KNRM); \
  }

#endif // ZP3_TEST_HPP
