#include <stdio.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <stdbool.h>

#define POOL_IMPLEMENTATION
#include "pool.h"

bool is_power_of_2(int n) {
  if (n == 0) { return 0; }
  while (n != 1) {
    if (n % 2 != 0) {
      return 0;
    } else {
      n = n / 2;
    }
  }
  return 1;
}

void usage(char *progName) {
  fprintf(stderr, "Usage: %s [-hpr] [-f FMT] [-b BATCH] [-n NUMBATCHES] -d DIR REPORT\n", progName);
  fprintf(stderr, "   -h get extended help\n");
  fprintf(stderr, "   -d DIR: A directory with pool configuration files\n");
  fprintf(stderr, "   Options for the possibilities ('poss') report:\n");
  fprintf(stderr, "   -p show progress and ETA for possibilities report\n");
  fprintf(stderr, "   -r generate possibilities report from saved bin files\n");
  fprintf(stderr, "   -f FMT: Format of report, one of 'text', 'bin' or 'json'\n");
  fprintf(stderr, "   -b BATCH: Batch number for multi process possibilities, should be less than numBatches\n");
  fprintf(stderr, "   -n NUMBATCHES: Number of batches, must be a power of 2\n");
  fprintf(stderr, "   REPORT: report to run\n");
  fprintf(stderr, "     teams: show configured teams\n");
  fprintf(stderr, "     results: show tournament bracket\n");
  fprintf(stderr, "     entries: show tournament entry brackets\n");
  fprintf(stderr, "     scores: show current scores\n");
  fprintf(stderr, "     poss: show possiblities stats scores.\n");
}

void help(void) {
  fprintf(stderr, "\nThe pool configuration directory DIR must contain the following:\n");
  fprintf(stderr, "     config.txt: file containing basic configuration in name=value format\n");
  fprintf(stderr, "          name: The name of the pool, default: 'NCAA Tournament'\n");
  fprintf(stderr, "          scorerType: One of Basic, Upset, SeedDiff or JoshP (see Scorers below)\n");
  fprintf(stderr, "          roundScores: 6 numeric round scores for the Scorer (See below)\n");
  fprintf(stderr, "     teams.txt: file containing the teams in the tournament in matchup order\n");
  fprintf(stderr, "          1 team per line in the form 'Name,shortName'\n");
  fprintf(stderr, "              name can be at most 32 chars, short name 3 chars\n");
  fprintf(stderr, "          teams should be listed by region in seed matchup order:\n");
  fprintf(stderr, "              1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15\n");
  fprintf(stderr, "          region 1 first, region 2 second, etc\n");
  fprintf(stderr, "          final four: region 1 vs region 2, region 3 vs. region 4\n");
  fprintf(stderr, "     results.txt: file containing short names of winners of each match\n");
  fprintf(stderr, "          winners can be reported in any order, EXCEPT:\n");
  fprintf(stderr, "          all current round winners must be reported before next round winners may be reported\n");
  fprintf(stderr, "          lines starting with '#' are ignored and may contain comments or notes\n");
  fprintf(stderr, "          last line should be the actual total score of the championship\n");
  fprintf(stderr, "     entries: a directory containing entry files\n");
  fprintf(stderr, "          entry files are named for the entrant\n");
  fprintf(stderr, "          same format as the results.txt file, except last line is the predicted\n");
  fprintf(stderr, "          total points of the championship and is used to break ties.\n");
  fprintf(stderr, "\nScorers:\n");
  fprintf(stderr, "Each scorer uses the configured roundScores as follows:\n");
  fprintf(stderr, "    Basic: each correct pick is worth the roundScore of the round\n");
  fprintf(stderr, "    Upset: each correct pick is worth the roundScore plus the seed of the winner\n");
  fprintf(stderr, "    JoshP: each correct pick is worth the roundScore multiplied by the seed of the winner\n");
  fprintf(stderr, " SeedDiff: each correct pick is worth the roundScore. If the loser is also picked correctly,\n");
  fprintf(stderr, "           the difference between the winner's seed and loser's seed is added if > 0\n");
  fprintf(stderr, "\nExample Configuration File:\n");
  fprintf(stderr, "name=Supercalifragilistic Pool\nscorerType=Upset\nroundScores=1,2,4,8,11,15\n");
  fprintf(stderr, "\nSample teams.txt file: https://github.com/seifertd/tournament3/blob/main/2023/teams.txt\n");
  fprintf(stderr, "\nSample results.txt file: https://github.com/seifertd/tournament3/blob/main/2023/results.txt\n");
}

int main(int argc, char *argv[]) {
  int opt;
  bool initialized = false;
  PoolReportFormat format = PoolFormatText;
  bool progress = false;
  bool restore = false;
  int batch = 0;
  int numBatches = 1;
  while ((opt = getopt(argc, argv, "d:f:b:n:prh")) != -1) {
    switch (opt) {
      case 'r':
        restore = true;
        break;
      case 'p':
        progress = true;
        break;
      case 'd':
        pool_initialize(optarg);
        initialized = true;
        break;
      case 'f':
        format = pool_str_to_format(optarg);
        if (format == PoolFormatInvalid) {
          usage(argv[0]);
          exit(EXIT_FAILURE);
        }
        break;
      case 'h':
        usage(argv[0]);
        help();
        exit(EXIT_FAILURE);
      case 'b':
        batch = atoi(optarg);
        break;
      case 'n':
        numBatches = atoi(optarg);
        if (!is_power_of_2(numBatches)) {
          fprintf(stderr, "[ERROR] numBatches %d is not a power of 2\n", numBatches);
          usage(argv[0]);
          exit(EXIT_FAILURE);
        }
        break;
      default:
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
  }
  if (batch >= numBatches) {
    fprintf(stderr, "[ERROR] batch %d is not < numBatches %d\n",
            batch, numBatches);
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  if (!initialized) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  if (optind >= argc) {
    fprintf(stderr, "Please specify a REPORT.\n");
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }
  char * command = argv[optind];
  if (strcmp(command, "teams") == 0) {
    pool_team_report();
  } else if (strcmp(command, "scores") == 0) {
    pool_score_report();
  } else if (strcmp(command, "poss") == 0) {
    pool_possibilities_report(format, progress, batch, numBatches, restore);
  } else if (strcmp(command, "entries") == 0) {
    pool_entries_report();
  } else if (strcmp(command, "results") == 0) {
    printf("%s: Results\n", poolConfiguration.poolName);
    pool_print_entry(&poolTournamentBracket);
  } else {
    fprintf(stderr, "Unknown REPORT: %s\n", command);
    usage(argv[0]);
  }
  return 0;
}
