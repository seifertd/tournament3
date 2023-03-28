#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define POOL_IMPLEMENTATION
#include "pool.h"

void usage(char *progName) {
  fprintf(stderr, "Usage: %s [-h] -d DIR COMMAND\n", progName);
  fprintf(stderr, "   -h get extended help\n");
  fprintf(stderr, "   -d DIR: A directory with pool configuration files\n");
  fprintf(stderr, "   COMMAND: report to run\n");
  fprintf(stderr, "     teams: show configured teams\n");
  fprintf(stderr, "     results: show tournament bracket\n");
  fprintf(stderr, "     entries: show tournament entry brackets\n");
  fprintf(stderr, "     scores: show current scores\n");
  fprintf(stderr, "     poss: show possiblities stats scores. Don't run until round 1 is over.\n");
}

void help() {
  fprintf(stderr, "\nThe pool configuration directory DIR must contain the following:\n");
  fprintf(stderr, "     config.txt: file containing basic configuration in name=value format\n");
  fprintf(stderr, "          name: The name of the pool, default: 'NCAA Tournament'\n");
  fprintf(stderr, "          scorerType: One of Basic, Upset or SeedDiff (see Scorers below)\n");
  fprintf(stderr, "          roundMultipliers: 6 numeric round multipliers for the Scorer (See below)\n");
  fprintf(stderr, "     teams.txt: file containing the teams in the tournament in matchup order\n");
  fprintf(stderr, "          1 team per line in the form 'Name,shortName'\n");
  fprintf(stderr, "              name can be at most 32 chars, short name 3 chars\n");
  fprintf(stderr, "          teams should be listed by region in seed matchup order:\n");
  fprintf(stderr, "              1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15\n");
  fprintf(stderr, "          region 1 first, region 2 second, etc\n");
  fprintf(stderr, "     results.txt: file containing short names of winners of each match\n");
  fprintf(stderr, "          winners can be reported in any order, EXCEPT:\n");
  fprintf(stderr, "          all round winners must be reported before new round winners may be reported\n");
  fprintf(stderr, "          lines starting with '#' are ignored and may contain comments or notes\n");
  fprintf(stderr, "     entries: a directory containing entry files\n");
  fprintf(stderr, "          entry files are named for the entrant\n");
  fprintf(stderr, "          same format as the results.txt file\n");
  fprintf(stderr, "\nScorers:\n");
  fprintf(stderr, "There are 3 supported scorers. Each scorer uses the configured\n");
  fprintf(stderr, "roundMultipliers as follows:\n");
  fprintf(stderr, "    Basic: each correct pick is worth the roundMultiplier of the round\n");
  fprintf(stderr, "           the game was played in.\n");
  fprintf(stderr, "    Upset: same as Basic except the seed number of the victor is added\n");
  fprintf(stderr, "    SeedDiff: same as Basic except in the case of upsets, the DIFFERENCE\n");
  fprintf(stderr, "           between the winner and loser seeds is added\n");
  fprintf(stderr, "\nExample Configuration File:\n");
  fprintf(stderr, "name=Supercalifragilistic Pool\nscorerType=Upset\nroundMultipliers=1,2,4,8,11,15\n");
  fprintf(stderr, "\nSample teams.txt file: https://github.com/seifertd/tournament3/blob/main/2023/teams.txt\n");
  fprintf(stderr, "\nSample results.txt file: https://github.com/seifertd/tournament3/blob/main/2023/results.txt\n");
}

int main(int argc, char *argv[]) {
  int opt;
  bool initialized = false;
  char *format = "text";
  while ((opt = getopt(argc, argv, "d:f:h")) != -1) {
    switch (opt) {
      case 'd':
        pool_initialize(optarg);
        initialized = true;
        break;
      case 'f':
        format = optarg;
        break;
      case 'h':
        usage(argv[0]);
        help();
        exit(EXIT_FAILURE);
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  if (!initialized) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  if (optind >= argc) {
    fprintf(stderr, "Please specify a COMMAND.\n");
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  char * command = argv[optind];
  if (strcmp(command, "teams") == 0) {
    pool_team_report();
  } else if (strcmp(command, "scores") == 0) {
    pool_score_report();
  } else if (strcmp(command, "poss") == 0) {
    printf("FORMAT: %s\n", format);
    pool_possibilities_report();
  } else if (strcmp(command, "entries") == 0) {
    pool_entries_report();
  } else if (strcmp(command, "results") == 0) {
    printf("%s: Results\n", poolConfiguration.poolName);
    pool_print_entry(&poolTournamentBracket);
  } else {
    fprintf(stderr, "Unknown COMMAND: %s\n", command);
    usage(argv[0]);
  }
  return 0;
}
