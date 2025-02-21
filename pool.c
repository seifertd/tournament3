#include <stdio.h>
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
  fprintf(stderr, "     poss: show possibilities stats\n");
  fprintf(stderr, "     ffour: show final four possibilities\n");
}

void help(void) {
  fprintf(stderr, "\n");
  /* BEGIN HELP */
  fprintf(stderr,
"Instructions\n"
"\n"
"1.  Create a directory on your computer to hold the pool files.\n"
"2.  Create the config.txt file and set your pool’s name, scoring method\n"
"    and round multipliers.\n"
"    1.  If you are running this pool for profit, also fill in the fee\n"
"        and payouts lines.\n"
"    2.  The fee is the number of currency units you collect per entry.\n"
"    3.  The payouts line is a comma separated list of no more than 4\n"
"        integers which specify the payouts by rank.\n"
"        1.  positive numbers specify a percentage of the total fees\n"
"            collected\n"
"        2.  -1 specifies that rank gets the entry fee back.\n"
"        3.  The sum of the percentages has to equal exactly 100\n"
"    4.  Sample config.txt file\n"
"3.  Create the teams.txt file in the directory.\n"
"    1.  Each line is in the form “Long Name,3-character Short Name”\n"
"    2.  Each team must have a unique short name.\n"
"    3.  The short names are case sensitive.\n"
"    4.  For the four play in games, use PI1, PI2, PI3 and PI4 as the\n"
"        short name. The long name is irrelevant.\n"
"    5.  The order of teams listed is as follows: top left region first,\n"
"        bottom left region second, top right region third, bottom right\n"
"        region fourth. The left side plays the right side in the\n"
"        championship. Within each region, teams are in order of seed as\n"
"        follows: 1, 16, 8, 9, 5, 12, 4, 13, 6, 11, 3, 14, 7, 10, 2, 15\n"
"    6.  Lines starting with a ‘#’ character are considered comments and\n"
"        ignored\n"
"    7.  Sample teams.txt file\n"
"4.  Collect entries\n"
"    1.  You can hand out forms, set up a pool on one of the free\n"
"        websites or send out the included bracket HTML entry collector\n"
"        page via email.\n"
"    2.  The HTML entry collector needs to be edited with the current\n"
"        year’s teams.\n"
"        1.  New for 2024: there is a ruby script to generate the web\n"
"            entry collector from the\n"
  );
  fprintf(stderr,
" teams file. You need a working ruby\n"
"            install and can run it as follows:\n"
"            ruby web/make_bracket.rb ./2024/teams.txt ./web/2024_bracket.html ./2024/logo.uri\n"
"            -   1st argument: the location of the generator script in\n"
"                this repo\n"
"            -   2nd argument: the location of the teams file\n"
"            -   3rd argument: location where to save the bracket\n"
"            -   4th argument: the location of a logo image file in\n"
"                urldata format.\n"
"        2.  NOTE: The teams file MUST contain a comment containing the\n"
"            region name in front of each bracket of teams. NO OTHER\n"
"            comments are allowed. See the sample teams.txt file above\n"
"            for correct format.\n"
"    3.  Here is what the HTML entry collector looks like: Latest NCAA\n"
"        Tournament Bracket\n"
"5.  Create the entries subfolder and copy/create/save any entry files\n"
"    you got from the pool entrants.\n"
"6.  Generate an entries report and send it out to everyone for\n"
"    confirmation: ./pool -d mypool entries\n"
"7.  Handle the play in games.\n"
"    1.  As the play in games are played, add to the pool config.txt file\n"
"        lines that map the place holder short team names (PI[1234]) with\n"
"        a new short name matching the winner. You will end up with lines\n"
"        like:\n"
"\n"
"            PI1=Wag\n"
"            PI2=Cdo\n"
"            PI3=GrS\n"
"            PI4=CSt\n"
"\n"
"    2.  Make sure to change the long name of the play in teams in the\n"
"        teams.txt file, but don’t modify the PI[1234] short names.\n"
"8.  As games are played in the tournament, record the winners in the\n"
"    results.txt file, one team short name per line. For play in games,\n"
"    you can use either the original PI[1234] short names or the mapped\n"
"    short names of the play in games from the step above.\n"
"    1.  Sample results.txt file\n"
"9.  Run the scores report: ./pool -d mypool scores until the \n"
  );
  fprintf(stderr,
"first round\n"
"    is complete.\n"
"10. Run your first possibilities report as soon as your machine can\n"
"    handle it: ./pool -d mypool poss and pass around the results via\n"
"    email, slack, discord or whatever.\n"
"11. When the final four teams are determined, run the Final Four report\n"
"    to show all top 4 standings and payouts (if configured) for each of\n"
"    the remaining possibilities: ./pool -d mypool ffour\n"
"\n"
"Scorers\n"
"\n"
"The pool configuration specifies the scorer to use in the pool. Each\n"
"scorer uses as input the bracket being scored, the round number and the\n"
"game number. In addition to the scorer, the pool configuration specifies\n"
"6 round multipliers that the scorer can use. The default multipliers are\n"
"1, 2, 4, 8, 16, 32. To override the default, supply a roundScores\n"
"configuration option in the config.txt file:\n"
"\n"
"roundScores=1,2,4,8,12,24\n"
"\n"
"These are the supported scorers:\n"
"\n"
"1.  Basic: each correct pick is worth a constant amount - the round\n"
"    multiplier configured for that round.\n"
"2.  Upset: each correct pick is worth the round multiplier for the\n"
"    game’s round plus the seed number of the winning team.\n"
"3.  SeedDiff: each correct pick is worth the round multiplier for the\n"
"    game’s round plus the difference in seeds of the winning team and\n"
"    the losing team. The bonus points only apply if the loser was picked\n"
"    correctly and the winner’s seed is greater than the loser’s seed.\n"
"4.  RelaxedSeedDiff: each correct pick is worth the round multiplier for\n"
"    the game’s round plus the difference in seeds of the winning team\n"
"    and the losing team. The bonus points only apply if winner’s seed is\n"
"    greater than the loser’s seed. This differs from SeedDiff in that\n"
"    the loser in the game need not have been picked correctly.\n"
"5.  JoshP: each correct pick is worth the round multiplier for the\n"
"    game’s round multiplied by the seed number of the winning team.\n"
"\n"
  );
  /* END HELP */
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
  } else if (strcmp(command, "ffour") == 0) {
    pool_final_four_report();
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
