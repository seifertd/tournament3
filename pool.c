#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define POOL_IMPLEMENTATION
#include "pool.h"

void usage(char *progName) {
  fprintf(stderr, "Usage: %s -d DIR\n", progName);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int opt;
  bool initialized = false;
  while ((opt = getopt(argc, argv, "d:")) != -1) {
    switch (opt) {
      case 'd':
        pool_initialize(optarg);
        initialized = true;
        break;
      default:
        usage(argv[0]);
    }
  }
  if (!initialized) {
    usage(argv[0]);
  }
  pool_team_report();
  pool_score_report();
  return 0;
}
