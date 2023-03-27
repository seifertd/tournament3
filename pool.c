#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define POOL_IMPLEMENTATION
#include "pool.h"

void usage(char *progName) {
  fprintf(stderr, "Usage: %s -d DIR COMMAND\n", progName);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
  int opt;
  bool initialized = false;
  char *format = "text";
  while ((opt = getopt(argc, argv, "d:f:")) != -1) {
    switch (opt) {
      case 'd':
        pool_initialize(optarg);
        initialized = true;
        break;
      case 'f':
        format = optarg;
        break;
      default:
        usage(argv[0]);
    }
  }
  if (!initialized) {
    usage(argv[0]);
  }
  if (optind >= argc) {
    fprintf(stderr, "Please specify a COMMAND.\n");
    usage(argv[0]);
  }
  char * command = optind < argc ? argv[optind] : "NULL";
  if (strcmp(command, "teams") == 0) {
    pool_team_report();
  } else if (strcmp(command, "scores") == 0) {
    pool_score_report();
  } else if (strcmp(command, "poss") == 0) {
    printf("FORMAT: %s\n", format);
    pool_possibilities_report();
  } else if (strcmp(command, "entries") == 0) {
    pool_print_entry(&poolTournamentBracket);
    pool_entries_report();
  } else {
    fprintf(stderr, "Unknown COMMAND: %s\n", command);
    usage(argv[0]);
  }
  return 0;
}
