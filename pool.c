#ifndef POOL_C_
#define POOL_C_
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

#ifndef POOLDEF
#define POOLDEF static inline
#endif

// Public API
#define POOL_NUM_TEAMS 64
#define POOL_NUM_GAMES 63
#define POOL_TEAM_NAME_LIMIT 33
#define POOL_BRACKET_NAME_LIMIT 33
#define POOL_TEAM_SHORT_NAME_LIMIT 4
#define POOL_NAME_LIMIT 256
#define POOL_ROUNDS 6
typedef struct {
  char name[POOL_TEAM_NAME_LIMIT];
  char shortName[POOL_TEAM_SHORT_NAME_LIMIT];
  uint8_t seed;
} PoolTeam;
static PoolTeam poolTeams[POOL_NUM_TEAMS] = {0};
static uint8_t poolSeeds[POOL_NUM_TEAMS] = {
  1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15,
  1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15,
  1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15,
  1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15
};
static uint8_t poolGameNumber[POOL_NUM_TEAMS][POOL_ROUNDS] = {
  {0, 32, 48, 56, 60, 62},
  {0, 32, 48, 56, 60, 62},
  {1, 32, 48, 56, 60, 62},
  {1, 32, 48, 56, 60, 62},
  {2, 33, 48, 56, 60, 62},
  {2, 33, 48, 56, 60, 62},
  {3, 33, 48, 56, 60, 62},
  {3, 33, 48, 56, 60, 62},
  {4, 34, 49, 56, 60, 62},
  {4, 34, 49, 56, 60, 62},
  {5, 34, 49, 56, 60, 62},
  {5, 34, 49, 56, 60, 62},
  {6, 35, 49, 56, 60, 62},
  {6, 35, 49, 56, 60, 62},
  {7, 35, 49, 56, 60, 62},
  {7, 35, 49, 56, 60, 62},
  {8, 36, 50, 57, 60, 62},
  {8, 36, 50, 57, 60, 62},
  {9, 36, 50, 57, 60, 62},
  {9, 36, 50, 57, 60, 62},
  {10, 37, 50, 57, 60, 62},
  {10, 37, 50, 57, 60, 62},
  {11, 37, 50, 57, 60, 62},
  {11, 37, 50, 57, 60, 62},
  {12, 38, 51, 57, 60, 62},
  {12, 38, 51, 57, 60, 62},
  {13, 38, 51, 57, 60, 62},
  {13, 38, 51, 57, 60, 62},
  {14, 39, 51, 57, 60, 62},
  {14, 39, 51, 57, 60, 62},
  {15, 39, 51, 57, 60, 62},
  {15, 39, 51, 57, 60, 62},
  {16, 40, 52, 58, 61, 62},
  {16, 40, 52, 58, 61, 62},
  {17, 40, 52, 58, 61, 62},
  {17, 40, 52, 58, 61, 62},
  {18, 41, 52, 58, 61, 62},
  {18, 41, 52, 58, 61, 62},
  {19, 41, 52, 58, 61, 62},
  {19, 41, 52, 58, 61, 62},
  {20, 42, 53, 58, 61, 62},
  {20, 42, 53, 58, 61, 62},
  {21, 42, 53, 58, 61, 62},
  {21, 42, 53, 58, 61, 62},
  {22, 43, 53, 58, 61, 62},
  {22, 43, 53, 58, 61, 62},
  {23, 43, 53, 58, 61, 62},
  {23, 43, 53, 58, 61, 62},
  {24, 44, 54, 59, 61, 62},
  {24, 44, 54, 59, 61, 62},
  {25, 44, 54, 59, 61, 62},
  {25, 44, 54, 59, 61, 62},
  {26, 45, 54, 59, 61, 62},
  {26, 45, 54, 59, 61, 62},
  {27, 45, 54, 59, 61, 62},
  {27, 45, 54, 59, 61, 62},
  {28, 46, 55, 59, 61, 62},
  {28, 46, 55, 59, 61, 62},
  {29, 46, 55, 59, 61, 62},
  {29, 46, 55, 59, 61, 62},
  {30, 47, 55, 59, 61, 62},
  {30, 47, 55, 59, 61, 62},
  {31, 47, 55, 59, 61, 62},
  {31, 47, 55, 59, 61, 62}
};

typedef struct {
  uint8_t winners[POOL_NUM_TEAMS];
  uint8_t tieBreak;
  char name[POOL_BRACKET_NAME_LIMIT];
  uint32_t score;
  uint32_t maxScore;
  uint32_t roundScores[POOL_ROUNDS];
} PoolBracket;
#define POOL_BRACKET_CAPACITY 1024
static uint32_t pool_brackets_count = 0;
static PoolBracket poolBrackets[POOL_BRACKET_CAPACITY] = {0};
static PoolBracket poolTournamentBracket = {
  .winners = {0},
  .tieBreak = 0,
  .name = "Tourney",
  .score = 0,
  .maxScore = 0,
  .roundScores = {0}
};
#define POOL_CONFIG_FILE_NAME "config.txt"
#define POOL_TEAMS_FILE_NAME "teams.txt"
#define POOL_RESULTS_FILE_NAME "results.txt"
#define POOL_ENTRIES_DIR_NAME "entries"

typedef enum { PoolScorerBasic,  PoolScorerUpset = 2 } PoolScorerType;

POOLDEF void pool_initialize(const char *dirPath);
POOLDEF void pool_add_entries_in_dir(const char *dirPath);
POOLDEF void pool_entries_report();
POOLDEF void pool_score_report();
POOLDEF void pool_print_entry(PoolBracket *bracket);
POOLDEF void pool_read_config_file(const char *filePath);
POOLDEF void pool_read_team_file(const char *filePath);
POOLDEF uint8_t pool_team_num_for_short_name(const char *shortName);
POOLDEF uint8_t pool_read_entry_to_bracket(const char *filePath, const char *entryName, size_t entryNameSize, PoolBracket *bracket);
POOLDEF void pool_bracket_score(PoolBracket *bracket, PoolScorerType scorerType,
    uint32_t roundMultipliers[POOL_ROUNDS]);
POOLDEF uint8_t pool_round_of_game(uint8_t gameNum);
POOLDEF uint32_t pool_basic_scorer(uint8_t round, uint8_t winner, uint32_t roundMultipliers[POOL_ROUNDS]);
POOLDEF uint32_t pool_upset_scorer(uint8_t round, uint8_t winner, uint32_t roundMultipliers[POOL_ROUNDS]);
typedef uint32_t (*PoolScorerFunction)(uint8_t, uint8_t, uint32_t*);
POOLDEF PoolScorerFunction pool_get_scorer_function(PoolScorerType scorerType);

#define POOL_TEAM_SHORT_NAME(w) ( w == 0 ? "Unk" : poolTeams[w-1].shortName )
#define POOL_TEAM_NAME(w) ( w == 0 ? "Unknown" : poolTeams[w-1].name )
#define POOL_LOSER(w, g) ( w == 0 ? 0 : (g*2 + 1 - ((w-1) - g*2) + 1) )

typedef struct {
  uint32_t roundMultipliers[POOL_ROUNDS];
  PoolScorerType scorerType;
  uint32_t payouts[3];
  char poolName[POOL_NAME_LIMIT];
} PoolConfiguration;
static PoolConfiguration poolConfiguration = {0};

#endif // POOL_C_ IMPLEMENTATION

#ifdef POOL_IMPLEMENTATION
POOLDEF uint32_t pool_basic_scorer(uint8_t round, uint8_t winner, uint32_t roundMultipliers[POOL_ROUNDS]) {
  return roundMultipliers[round];
}
POOLDEF uint32_t pool_upset_scorer(uint8_t round, uint8_t winner, uint32_t roundMultipliers[POOL_ROUNDS]) {
  PoolTeam *winningTeam = &poolTeams[winner-1];
  return roundMultipliers[round] + winningTeam->seed;
}
POOLDEF PoolScorerFunction pool_get_scorer_function(PoolScorerType scorerType) {
  switch(scorerType) {
    case PoolScorerBasic:
      return &pool_basic_scorer;
      break;
    case PoolScorerUpset:
      return &pool_upset_scorer;
      break;
    default:
      fprintf(stderr, "Error, unknown PoolScorerType: %d\n", scorerType);
      exit(1);
  }
}

POOLDEF uint8_t pool_round_of_game(uint8_t gameNum) {
  static uint8_t gamesInRound[POOL_ROUNDS] = {32, 48, 56, 60, 62, 63};
  uint8_t r = 0;
  while(r < POOL_ROUNDS && gameNum < gamesInRound[r]) { r++; };
  return r;
}

POOLDEF void pool_bracket_score(PoolBracket *bracket, PoolScorerType scorerType,
    uint32_t roundMultipliers[POOL_ROUNDS]) {
  static uint8_t gamesInRound[POOL_ROUNDS] = {32, 48, 56, 60, 62, 63};
  bracket->score = 0;
  bracket->maxScore = 0;
  bracket->roundScores[0] = 0;
  uint8_t round = 0;
  PoolScorerFunction scorerFunc = pool_get_scorer_function(scorerType);
  for (uint8_t g = 0; g < POOL_NUM_GAMES; g++) {
    if (g >= gamesInRound[round]) {
      round++;
      bracket->roundScores[round] = 0;
    }
    uint8_t bracketWinner = bracket->winners[g];
    uint8_t tourneyWinner = poolTournamentBracket.winners[g];
    uint32_t gameScore = (*scorerFunc)(round, bracketWinner, roundMultipliers);
    if (tourneyWinner != 0) {
      if (bracketWinner == tourneyWinner) {
        bracket->score += gameScore;
        bracket->maxScore += gameScore;
        bracket->roundScores[round] += gameScore;
      }
    } else {
      bracket->maxScore += gameScore;
    }
  }
}

POOLDEF void pool_initialize(const char *dirPath) {
  struct dirent *dp;
  DIR *dfd;

  if ((dfd = opendir(dirPath)) == NULL) {
    fprintf(stderr, "Can't open directory %s: %s\n",
        dirPath, strerror(errno));
    exit(1);
  }

  char filePath[1024];

  // Read configuration
  sprintf(filePath, "%s/%s", dirPath, POOL_CONFIG_FILE_NAME);
  pool_read_config_file(filePath);

  // Read teams
  sprintf(filePath, "%s/%s", dirPath, POOL_TEAMS_FILE_NAME);
  pool_read_team_file(filePath);

  // Read entries
  sprintf(filePath, "%s/%s", dirPath, POOL_ENTRIES_DIR_NAME);
  pool_add_entries_in_dir(filePath);

  // Read results
  sprintf(filePath, "%s/%s", dirPath, POOL_RESULTS_FILE_NAME);
  uint8_t gamesPlayed = pool_read_entry_to_bracket(filePath, "Tourney", 7, &poolTournamentBracket);

  closedir(dfd);
}

POOLDEF void pool_print_entry(PoolBracket *bracket) {
  printf("%10.10s\n", bracket->name);
  printf("   ");
  for (size_t g = 0; g < 32; g++) {
    uint8_t loser = POOL_LOSER(bracket->winners[g], g);
    if (g == 0) {
      printf("%3s", POOL_TEAM_SHORT_NAME(loser));
    } else {
      printf(" %3s", POOL_TEAM_SHORT_NAME(loser));
    }
  }
  printf("\n");
  printf("1: ");
  for (size_t g = 0; g < 32; g++) {
    if (g == 0) {
      printf("%3s", POOL_TEAM_SHORT_NAME(bracket->winners[g]));
    } else {
      printf(" %3s", POOL_TEAM_SHORT_NAME(bracket->winners[g]));
    }
  }
  printf("\n");
  printf("2: ");
  for (size_t g = 0; g < 16; g++) {
    if (g == 0) {
      printf("%2s%-5s", "", POOL_TEAM_SHORT_NAME(bracket->winners[g + 32]));
    } else {
      printf(" %2s%-5s", "", POOL_TEAM_SHORT_NAME(bracket->winners[g + 32]));
    }
  }
  printf("\n");
  printf("3: ");
  for (size_t g = 0; g < 8; g++) {
    if (g == 0) {
      printf("%6s%-9s", "", POOL_TEAM_SHORT_NAME(bracket->winners[g + 48]));
    } else {
      printf(" %6s%-9s", "", POOL_TEAM_SHORT_NAME(bracket->winners[g + 48]));
    }
  }
  printf("\n");
  printf("4: ");
  for (size_t g = 0; g < 4; g++) {
    if (g == 0) {
      printf("%14s%-17s", "", POOL_TEAM_SHORT_NAME(bracket->winners[g + 56]));
    } else {
      printf(" %14s%-17s", "", POOL_TEAM_SHORT_NAME(bracket->winners[g + 56]));
    }
  }
  printf("\n");
  printf("5: ");
  for (size_t g = 0; g < 2; g++) {
    if (g == 0) {
      printf("%30s%-32s", "", POOL_TEAM_SHORT_NAME(bracket->winners[g + 60]));
    } else {
      printf(" %30s%-32s", "", POOL_TEAM_SHORT_NAME(bracket->winners[g + 60]));
    }
  }
  printf("\n");
  printf("6: Champion: %s Tie Breaker: %d\n", POOL_TEAM_NAME(bracket->winners[62]), bracket->tieBreak);
  printf("\n\n");
}

POOLDEF int pool_score_cmpfunc (const void * a, const void * b) {
   return ( ((PoolBracket*)b)->score - ((PoolBracket*)a)->score );
}

POOLDEF void pool_score_report() {
  if (pool_brackets_count == 0) {
    fprintf(stderr, "There are no entries in this pool.");
  }
  for (size_t i = 0; i < pool_brackets_count; i++) {
    PoolBracket *bracket = &poolBrackets[i];
    pool_bracket_score(bracket,
        poolConfiguration.scorerType,
        poolConfiguration.roundMultipliers);
  }
  qsort(poolBrackets, pool_brackets_count, sizeof(PoolBracket), pool_score_cmpfunc);
  printf("            Curr  Max            Round\n");
  printf("  Name     Score Score   1   2   3   4   5   6\n");
  printf("---------- ----- ----- --- --- --- --- --- ---\n");
  for (size_t i = 0; i < pool_brackets_count; i++) {
    printf("%10.10s %5d %5d %3d %3d %3d %3d %3d %3d\n", poolBrackets[i].name,
        poolBrackets[i].score,
        poolBrackets[i].maxScore,
        poolBrackets[i].roundScores[0],
        poolBrackets[i].roundScores[1],
        poolBrackets[i].roundScores[2],
        poolBrackets[i].roundScores[3],
        poolBrackets[i].roundScores[4],
        poolBrackets[i].roundScores[5]
        );
  }
}

POOLDEF int pool_name_cmpfunc (const void * a, const void * b) {
   char alower[POOL_BRACKET_NAME_LIMIT];
   char blower[POOL_BRACKET_NAME_LIMIT];
   for(int i = 0; i < POOL_BRACKET_NAME_LIMIT; i++) alower[i] = tolower(((PoolBracket*)a)->name[i]);
   for(int i = 0; i < POOL_BRACKET_NAME_LIMIT; i++) blower[i] = tolower(((PoolBracket*)b)->name[i]);
   return strcmp(alower, blower);
}

POOLDEF void pool_entries_report() {
  if (pool_brackets_count == 0) {
    fprintf(stderr, "There are no entries in this pool.");
  }
  qsort(poolBrackets, pool_brackets_count, sizeof(PoolBracket), pool_name_cmpfunc);
  for (size_t i = 0; i < pool_brackets_count; i++) {
    pool_print_entry(&poolBrackets[i]);
  }
}

POOLDEF uint8_t pool_read_entry_to_bracket(const char *filePath, const char *entryName, size_t entryNameSize, PoolBracket *bracket) {
  FILE *f = fopen(filePath, "rb");
  if (f == NULL) {
    fprintf(stderr, "Could not open entry file %s: %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  if (entryNameSize > POOL_BRACKET_NAME_LIMIT) {
    fprintf(stderr, "Entry file name has to be less than %d chars long: %s\n",
        POOL_BRACKET_NAME_LIMIT, filePath);
    exit(1);
  }
  uint8_t buffer[1024];
  strncpy(bracket->name, entryName, entryNameSize);
  strncpy(bracket->name + entryNameSize, "", 1);
  char * line = fgets(buffer, 1023, f);
  uint8_t teamRounds[POOL_NUM_TEAMS] = {0};
  uint8_t resultsCount = 0;
  for (size_t i = 0; i < POOL_NUM_GAMES + 1 && line != NULL; i++) {
    // Turn the new line into a 0
    buffer[strlen(buffer)- 1] = 0;

    // Skip lines starting with '#'
    if (buffer[0] == '#') {
      continue;
    }

    if (i == POOL_NUM_GAMES) {
      // reading tie break
      bracket->tieBreak = (uint8_t) atoi(buffer);
    } else {
      uint8_t teamNum = pool_team_num_for_short_name(buffer);
      if (teamNum == 0) {
        fprintf(stderr, "Could not determine team number of %s\n", buffer);
        exit(1);
      }
      uint8_t round = teamRounds[teamNum-1];
      size_t gameIndex = poolGameNumber[teamNum-1][round];
      if (bracket->winners[gameIndex] != 0) {
        PoolTeam prevWinner = poolTeams[bracket->winners[gameIndex]-1];
        fprintf(stderr, "Error in bracket %s: conflicting result %s for game %ld, previous winner: %s\n",
            bracket->name, buffer, gameIndex, prevWinner.name);
        exit(1);
      } else {
        bracket->winners[gameIndex] = teamNum;
      }
      teamRounds[teamNum-1] = teamRounds[teamNum-1] + 1;
      line = fgets(buffer, 1023, f);
      resultsCount++;
    }
  }
  fclose(f);
  return resultsCount;
}

POOLDEF void pool_add_entries_in_dir(const char *dirPath) {
  struct dirent *dp;
  DIR *dfd;

  if ((dfd = opendir(dirPath)) == NULL) {
    fprintf(stderr, "Can't open directory %s: %s\n",
        dirPath, strerror(errno));
    exit(1);
  }

  char entryFilePath[1024];

  while ((dp = readdir(dfd)) != NULL) {
    struct stat stbuf;
    sprintf(entryFilePath, "%s/%s", dirPath, dp->d_name);
    if ( stat(entryFilePath, &stbuf) == -1 ) {
      // .. do nothing?
      continue;
    }
    if ( (stbuf.st_mode & S_IFMT ) == S_IFDIR ) {
      // skip directories
      continue;
    }
    char *ext = strrchr(entryFilePath, '.');
    if (!ext || strncmp(ext+1, "txt", 3) != 0) {
      // skip no extension or wrong extension
      continue;
    }
    PoolBracket *entry = &poolBrackets[pool_brackets_count++];
    uint8_t resultsAdded =
      pool_read_entry_to_bracket(entryFilePath, dp->d_name, strlen(dp->d_name) - 4, entry);
    if (resultsAdded != POOL_NUM_GAMES) {
      fprintf(stderr, "Entry file %s/%s is incomplete, only %d pick(s) made\n",
          dirPath, dp->d_name, resultsAdded);
      exit(1);
    }
  }
  closedir(dfd);
}

POOLDEF uint8_t pool_team_num_for_short_name(const char *shortName) {
  size_t len = strlen(shortName);
  assert(len <= POOL_TEAM_SHORT_NAME_LIMIT);
  for (size_t i = 0; i < POOL_NUM_TEAMS; i++) {
    if (strncmp(shortName, poolTeams[i].shortName, POOL_TEAM_SHORT_NAME_LIMIT) == 0) {
      return i + 1;
    }
  }
  return 0;
}

POOLDEF void pool_read_config_file(const char *filePath) {
  // Set defaults
  strcpy(poolConfiguration.poolName, "NCAA Tournament");
  poolConfiguration.roundMultipliers[0] = 1;
  poolConfiguration.roundMultipliers[1] = 2;
  poolConfiguration.roundMultipliers[2] = 4;
  poolConfiguration.roundMultipliers[3] = 8;
  poolConfiguration.roundMultipliers[4] = 16;
  poolConfiguration.roundMultipliers[5] = 32;
  poolConfiguration.scorerType = PoolScorerBasic;

  FILE *f = fopen(filePath, "rb");
  if (f == NULL) {
    // Do nothing, use defaults
    fprintf(stderr, "[WARN] No config.txt in pool directory, using defaults.\n");
    return;
  }
  uint8_t buffer[1024];
  bool readTitle, readMultipliers, readName = false;
  char * line = fgets(buffer, 1023, f);
  while (line != NULL) {
    buffer[strlen(buffer)- 1] = 0;
    if (strncmp(line, "name=", 5) == 0) {
      strncpy(poolConfiguration.poolName, line + 5, POOL_NAME_LIMIT);
    }
    if (strncmp(line, "roundMultipliers=", strlen("roundMultipliers=")) == 0) {
      if (sscanf(line, "roundMultipliers=%d,%d,%d,%d,%d,%d",
        &poolConfiguration.roundMultipliers[0],
        &poolConfiguration.roundMultipliers[1],
        &poolConfiguration.roundMultipliers[2],
        &poolConfiguration.roundMultipliers[3],
        &poolConfiguration.roundMultipliers[4],
        &poolConfiguration.roundMultipliers[5]) != 6) {
          fprintf(stderr, "config.txt roundMultipliers= line does not have 6 numerical values separated by commas\n");
          exit(1);
      }
    }
    if (strncmp(line, "scorerType=", 11) == 0) {
      if (strncmp(line+11, "Basic", 5) == 0) {
        // do nothing, default is basic
      } else if (strncmp(line+11, "Upset", 5) == 0) {
        poolConfiguration.scorerType = PoolScorerUpset;
      } else {
        fprintf(stderr, "config.txt scorerType %s is not known\n", line+11);
        exit(1);
      }
    }
    line = fgets(buffer, 1023, f);
  }
}

POOLDEF void pool_read_team_file(const char *filePath) {
  FILE *f = fopen(filePath, "rb");
  if (f == NULL) {
    fprintf(stderr, "Could not open team file %s: %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  uint8_t buffer[1024];
  for (size_t i = 0; i < POOL_NUM_TEAMS; i++) {
    if (fgets(buffer, 1023, f) == NULL) {
      fprintf(stderr, "Could not read a line from file %s: %s\n",
          filePath, strerror(errno));
      exit(1);
    }

    // Turn new line into end of string marker
    buffer[strlen(buffer)- 1] = 0;

    PoolTeam *team = &poolTeams[i];
    team->seed = poolSeeds[i];
    int vars_filled = sscanf(buffer, "%32[^,],%3s", team->name, team->shortName);
    if (vars_filled != 2) {
      fprintf(stderr, "Could not read name and short name from line %s\n", buffer);
      exit(1);
    }
  }
  fclose(f);
}
#endif // POOL_IMPLEMENTATION
