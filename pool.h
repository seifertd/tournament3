#ifndef POOL_C_
#define POOL_C_
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

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
#define POOL_MAX_PAYOUTS 4
#define POOL_FREQ_SIZE 4096
#define POOL_NUM_MODEL_STATS 24
#define POOL_STATS_FILE_NAME "stats.csv"
#define POOL_WEIGHTS_FILE_NAME "weights.json"

typedef struct {
  char name[POOL_TEAM_NAME_LIMIT];
  char shortName[POOL_TEAM_SHORT_NAME_LIMIT];
  uint8_t seed;
  bool eliminated;
} PoolTeam;
static PoolTeam poolTeams[POOL_NUM_TEAMS] = {0};
static uint8_t poolSeeds[POOL_NUM_TEAMS] = {
  1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15,
  1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15,
  1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15,
  1,16,8,9,5,12,4,13,6,11,3,14,7,10,2,15
};
static uint8_t poolGamesInRound[POOL_ROUNDS] = {32, 48, 56, 60, 62, 63};
static uint8_t poolGamesRound[POOL_NUM_GAMES] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,
  3,3,3,3,
  4,4,
  5
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
  uint8_t tieBreakDiff;
  char name[POOL_BRACKET_NAME_LIMIT];
  uint32_t score;
  uint32_t maxScore;
  uint32_t roundScores[POOL_ROUNDS];
  uint8_t wins;
} PoolBracket;
#define POOL_BRACKET_CAPACITY 1024
static uint32_t poolBracketsCount = 0;
static PoolBracket poolBrackets[POOL_BRACKET_CAPACITY] = {0};
static PoolBracket poolTournamentBracket = {
  .winners = {0},
  .tieBreak = 0,
  .tieBreakDiff = 0,
  .name = "Tourney",
  .score = 0,
  .maxScore = 0,
  .roundScores = {0},
  .wins = 0
};

typedef struct {
  uint16_t maxRank;
  uint16_t minRank;
  uint32_t maxScore;
  uint64_t champCounts[POOL_NUM_TEAMS];
  uint64_t timesWon;
  uint64_t timesTied;
  uint32_t possibleScore;
  PoolBracket *bracket;
} PoolStats;

typedef struct {
  PoolStats *stats;
  uint32_t possibleScore;
} PoolScoreStats;

typedef struct {
  PoolBracket *bracket;
  uint32_t possibleScore;
} PoolBracketScore;

#define POOL_CONFIG_FILE_NAME "config.txt"
#define POOL_TEAMS_FILE_NAME "teams.txt"
#define POOL_RESULTS_FILE_NAME "results.txt"
#define POOL_ENTRIES_DIR_NAME "entries"

typedef enum { PoolScorerBasic = 0,  PoolScorerUpset, PoolScorerJoshP, PoolScorerSeedDiff, PoolScorerRelaxedSeedDiff, PoolScorerUpsetMultiplier } PoolScorerType;
typedef enum { PoolFormatInvalid = 0, PoolFormatText, PoolFormatJson, PoolFormatBin } PoolReportFormat;
typedef enum { PoolMCSelectionSeedWeighted = 0, PoolMCSelectionModelWeighted } PoolMCSelectionMode;

POOLDEF void pool_defaults(void);
POOLDEF void pool_initialize(const char *dirPath);
POOLDEF void pool_team_report(void);
POOLDEF void pool_add_entries_in_dir(const char *dirPath);
POOLDEF void pool_entries_report(PoolReportFormat fmt);
POOLDEF void pool_score_report(PoolReportFormat fmt);
POOLDEF void pool_scoredetail_report(const char *entryName);
POOLDEF PoolReportFormat pool_str_to_format(const char *fmtStr);
POOLDEF void pool_possibilities_report(PoolReportFormat fmt, bool progress, int batch, int numBatches, bool restore);
POOLDEF void pool_monte_carlo_report(uint64_t numSamples, PoolReportFormat fmt, bool progress, PoolMCSelectionMode selectionMode);
POOLDEF bool pool_load_model_data(void);
POOLDEF void pool_final_four_report(PoolReportFormat fmt);
POOLDEF void pool_restore_stats_from_files(PoolStats stats[], uint32_t bracketCount);

POOLDEF void pool_advance_bracket_for_batch(PoolBracket *possibleBracket,
             uint8_t gamesLeft[], int *gamesLeftCount, int batch, int numBatches);
POOLDEF void pool_print_entry(PoolBracket *bracket);
POOLDEF void pool_read_config_file(const char *filePath);
POOLDEF void pool_read_team_file(const char *filePath);
POOLDEF uint8_t pool_games_played(void);
POOLDEF uint8_t pool_team_num_for_short_name(const char *shortName);
POOLDEF uint8_t pool_read_entry_to_bracket(const char *filePath, const char *entryName, size_t entryNameSize,
    PoolBracket *bracket, bool recordEliminations);
POOLDEF void pool_bracket_score(PoolBracket *bracket, PoolBracket *results);
POOLDEF uint8_t pool_round_of_game(uint8_t gameNum);
POOLDEF void pool_teams_of_game(uint8_t gameNum, uint8_t round, PoolBracket *bracket, uint8_t *team1, uint8_t *team2);
typedef uint32_t (*PoolScorerFunction)(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game);
POOLDEF uint32_t pool_basic_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game);
POOLDEF uint32_t pool_upset_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game);
POOLDEF uint32_t pool_josh_p_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game);
POOLDEF uint32_t pool_seed_diff_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game);
POOLDEF uint32_t pool_relaxed_seed_diff_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game);
POOLDEF uint32_t pool_upset_multiplier_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game);
POOLDEF uint32_t pool_dp_max_score(PoolBracket *bracket, PoolBracket *results);
POOLDEF PoolScorerFunction pool_get_scorer_function(PoolScorerType scorerType);
POOLDEF uint8_t pool_loser_of_game(uint8_t gameNum, PoolBracket *bracket);
POOLDEF void pool_print_humanized(FILE *f_stream, uint64_t num, int fieldLength);

#define STRIKE(w, pw, f) ( w == 0 ? f : (pw == 0 ? poolTeams[w-1].eliminated : w != pw ) ? "\033[9m\033[31m" f "\033[0m" : (pw != 0 ? "\033[32m" f "\033[0m" : f) )
#define POOL_TEAM_SHORT_NAME(w) ( w == 0 ? "Unk" : poolTeams[w-1].shortName )
#define POOL_TEAM_NAME(w) ( w == 0 ? "Unknown" : poolTeams[w-1].name )

typedef struct {
  uint32_t roundScores[POOL_ROUNDS];
  PoolScorerType scorerType;
  int payouts[POOL_MAX_PAYOUTS];
  uint8_t fee;
  char poolName[POOL_NAME_LIMIT];
  PoolScorerFunction poolScorer;
  char dirPath[1024];
  char pi1[POOL_TEAM_SHORT_NAME_LIMIT];
  char pi2[POOL_TEAM_SHORT_NAME_LIMIT];
  char pi3[POOL_TEAM_SHORT_NAME_LIMIT];
  char pi4[POOL_TEAM_SHORT_NAME_LIMIT];
  char regionNames[4][32];
} PoolConfiguration;
static PoolConfiguration poolConfiguration = {0};
static float poolTeamModelStats[POOL_NUM_TEAMS][POOL_NUM_MODEL_STATS] = {{0}};
static float poolModelWeights[POOL_NUM_MODEL_STATS] = {0};
static bool poolModelStatsLoaded = false;

#endif // POOL_C_ IMPLEMENTATION

#ifdef POOL_IMPLEMENTATION

#define UNUSED(x) (void)x

typedef struct {
  clock_t start;
  uint64_t total;
  uint64_t complete;
  uint8_t nextPercent;
} PoolProgress;

typedef struct {
  uint8_t team;
  uint64_t count;
} PoolTeamWins;

POOLDEF int LOG2(int n) {
  if (n <= 0) {
    return 0;
  }
  int val = -1;
  while (n) {
    val++;
    n >>= 1;
  }
  return val;
}

POOLDEF void pool_print_humanized(FILE *f_stream, uint64_t num, int fieldLength) {
  if (num < 10000) {
    fprintf(f_stream, "%*" PRIu64, fieldLength+1, num);
    return;
  }
  static uint64_t factors[6] = {1e18, 1e15, 1e12, 1e9, 1e6, 1e3};
  static char *abbrev[6] = {"Qn", "Qd", "T", "B", "M", "K"};
  for (int i = 0; i < 6; ++i) {
    if (factors[i] <= num) {
      double dispNum = (double) num / (double) factors[i];
      if (dispNum < 10) {
        fprintf(f_stream, "%*.3f%s", fieldLength, dispNum, abbrev[i]);
      } else if (dispNum < 100) {
        fprintf(f_stream, "%*.2f%s", fieldLength, dispNum, abbrev[i]);
      } else {
        if ((dispNum - (uint64_t) dispNum) >= 0.1) {
          fprintf(f_stream, "%*.1f%s", fieldLength, dispNum, abbrev[i]);
        } else {
          fprintf(f_stream, "%*.0f%s", fieldLength, dispNum, abbrev[i]);
        }
      }
      return;
    }
  }
}

POOLDEF void pool_inc_progress(PoolProgress *prog) {
  if (prog == NULL) {
    return;
  }
  prog->complete += 1;
  if (prog->nextPercent == 0) {
    fprintf(stderr, "DFS BPS: %12.0f 0%% ETA: --\r", 0.0);
    fflush(stderr);
    prog->nextPercent += 1;
  } else if (prog->complete < prog->total) {
    double perc = (double) prog->complete / (double) prog->total * 100.0;
    if ( perc > prog->nextPercent || prog->complete % 10000000 == 0) {
      clock_t curr = clock();
      double elapsed_t = (double)(curr - prog->start)/CLOCKS_PER_SEC;
      double bps = elapsed_t > 0 ? prog->complete / elapsed_t : 9999999.99;
      uint64_t eta = (uint64_t) ((prog->total - prog->complete) / bps);
      uint64_t hours = eta / 3600;
      uint64_t minutes = (eta - hours * 3600) / 60;
      uint64_t secs = eta % 60;
      fprintf(stderr, "DFS BPS: %12.0f %3.0f%% ETA: %02" PRIu64 ":%02" PRIu64 ":%02" PRIu64 "\r", bps, perc, hours, minutes, secs);
      fflush(stderr);
      prog->nextPercent += 1;
    }
  } else {
    clock_t curr = clock();
    uint64_t elapsed_t = (uint64_t)(curr - prog->start)/CLOCKS_PER_SEC;
    double bps = elapsed_t > 0 ? prog->complete / elapsed_t : 9999999.99;
    uint64_t hours = elapsed_t / 3600;
    uint64_t minutes = (elapsed_t - hours * 3600) / 60;
    uint64_t secs = elapsed_t % 60;
    fprintf(stderr, "DFS BPS: %12.0f %3.0f%% ELAPSED: %02" PRIu64 ":%02" PRIu64 ":%02" PRIu64 "\n", bps, 100.0, hours, minutes, secs);
    fflush(stderr);
  }
}

POOLDEF void pool_teams_of_game(uint8_t gameNum, uint8_t round, PoolBracket *bracket,
    uint8_t *team1, uint8_t *team2) {
  if (gameNum > POOL_NUM_GAMES - 1) {
    fprintf(stderr, "gameNum %d is too big in pool_loser_of_game\n", gameNum);
    exit(1);
  }
  if (round == 0) {
    *team1 = gameNum * 2 + 1;
    *team2 = *team1 + 1;
  } else {
    uint8_t gameBase = round == 1 ? 0 : poolGamesInRound[round - 2];
    uint8_t prevGame1 = gameBase + (gameNum - poolGamesInRound[round - 1]) * 2;
    uint8_t prevGame2 = prevGame1 + 1;
    *team1 = bracket->winners[prevGame1];
    *team2 = bracket->winners[prevGame2];
  }
  return;
}

POOLDEF uint8_t pool_loser_of_game(uint8_t gameNum, PoolBracket *bracket) {
  if (gameNum > POOL_NUM_GAMES - 1) {
    fprintf(stderr, "gameNum %d is too big in pool_loser_of_game\n", gameNum);
    exit(1);
  }
  uint8_t winner = bracket->winners[gameNum];
  if (winner == 0) { return 0; }
  uint8_t round = pool_round_of_game(gameNum);
  if (round == 0) {
    winner = winner - 1;
    return gameNum*2 + 1 - (winner - gameNum*2) + 1;
  }
  uint8_t gameBase = round == 1 ? 0 : poolGamesInRound[round - 2];
  uint8_t prevGame1 = gameBase + (gameNum - poolGamesInRound[round - 1]) * 2;
  uint8_t prevGame2 = prevGame1 + 1;
  return bracket->winners[prevGame1] == winner ? 
    bracket->winners[prevGame2] : bracket->winners[prevGame1];
}
POOLDEF uint32_t pool_basic_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game) {
  UNUSED(loser);
  if (bracket->winners[game] != winner) return 0;
  return poolConfiguration.roundScores[round];
}
POOLDEF uint32_t pool_seed_diff_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game) {
  if (bracket->winners[game] != winner) return 0;
  uint8_t bracketLoser = pool_loser_of_game(game, bracket);
  uint8_t bonus = 0;
  if (bracketLoser == loser) {
    uint8_t wSeed = poolTeams[winner - 1].seed;
    uint8_t lSeed = poolTeams[loser - 1].seed;
    bonus = wSeed > lSeed ? wSeed - lSeed : 0;
  }
  return poolConfiguration.roundScores[round] + bonus;
}
POOLDEF uint32_t pool_relaxed_seed_diff_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game) {
  if (bracket->winners[game] != winner) return 0;
  uint8_t wSeed = poolTeams[winner - 1].seed;
  uint8_t lSeed = poolTeams[loser - 1].seed;
  uint32_t bonus = wSeed > lSeed ? wSeed - lSeed : 0;
  return poolConfiguration.roundScores[round] + bonus;
}
POOLDEF uint32_t pool_upset_multiplier_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game) {
  if (bracket->winners[game] != winner) return 0;
  uint8_t wSeed = poolTeams[winner - 1].seed;
  uint8_t lSeed = poolTeams[loser - 1].seed;
  if (wSeed > lSeed) {
    return ceilf(poolConfiguration.roundScores[round] * 1.0 * wSeed / lSeed);
  }
  return poolConfiguration.roundScores[round];
}
// Computes exact max possible score using tree DP.
// dp[game][team] = max total bracket score from all games in this subtree,
// if team emerges as winner of game. UINT32_MAX = impossible.
POOLDEF uint32_t pool_dp_max_score(PoolBracket *bracket, PoolBracket *results) {
  uint32_t dp[POOL_NUM_GAMES][POOL_NUM_TEAMS];
  memset(dp, 0XFF, sizeof(dp));

  uint8_t round = 0;
  for (uint8_t g = 0; g < POOL_NUM_GAMES; g++) {
    if (g >= poolGamesInRound[round]) round++;

    if (round == 0) {
      uint8_t team1 = g * 2 + 1;
      uint8_t team2 = team1 + 1;

      if (results->winners[g] != 0) {
        uint8_t winner = results->winners[g];
        uint8_t loser = (winner == team1) ? team2 : team1;
        dp[g][winner - 1] = (*poolConfiguration.poolScorer)(bracket, winner, loser, 0, g);
      } else {
        if (!poolTeams[team1 - 1].eliminated) {
          dp[g][team1 - 1] = (*poolConfiguration.poolScorer)(bracket, team1, team2, 0, g);
        }
        if (!poolTeams[team2 - 1].eliminated) {
          dp[g][team2 - 1] = (*poolConfiguration.poolScorer)(bracket, team2, team1, 0, g);
        }
      }
    } else {
      uint8_t gameBase = round == 1 ? 0 : poolGamesInRound[round - 2];
      uint8_t prevGame1 = gameBase + (g - poolGamesInRound[round - 1]) * 2;
      uint8_t prevGame2 = prevGame1 + 1;

      if (results->winners[g] != 0) {
        uint8_t winner = results->winners[g];
        uint8_t loser = pool_loser_of_game(g, results);
        uint32_t dpW, dpL;
        if (dp[prevGame1][winner - 1] != UINT32_MAX) {
          dpW = dp[prevGame1][winner - 1];
          dpL = dp[prevGame2][loser - 1];
        } else {
          dpW = dp[prevGame2][winner - 1];
          dpL = dp[prevGame1][loser - 1];
        }
        uint32_t gameScore = (*poolConfiguration.poolScorer)(bracket, winner, loser, round, g);
        dp[g][winner - 1] = dpW + dpL + gameScore;
      } else {
        for (uint8_t tl = 0; tl < POOL_NUM_TEAMS; tl++) {
          if (dp[prevGame1][tl] == UINT32_MAX) continue;
          for (uint8_t tr = 0; tr < POOL_NUM_TEAMS; tr++) {
            if (dp[prevGame2][tr] == UINT32_MAX) continue;
            uint32_t subtreeScore = dp[prevGame1][tl] + dp[prevGame2][tr];

            // TL wins game g
            {
              uint32_t gameScore = (*poolConfiguration.poolScorer)(bracket, tl + 1, tr + 1, round, g);
              uint32_t candidate = subtreeScore + gameScore;
              if (dp[g][tl] == UINT32_MAX || candidate > dp[g][tl]) {
                dp[g][tl] = candidate;
              }
            }

            // TR wins game g
            {
              uint32_t gameScore = (*poolConfiguration.poolScorer)(bracket, tr + 1, tl + 1, round, g);
              uint32_t candidate = subtreeScore + gameScore;
              if (dp[g][tr] == UINT32_MAX || candidate > dp[g][tr]) {
                dp[g][tr] = candidate;
              }
            }
          }
        }
      }
    }
  }

  uint32_t maxScore = 0;
  for (uint8_t t = 0; t < POOL_NUM_TEAMS; t++) {
    if (dp[POOL_NUM_GAMES - 1][t] != UINT32_MAX && dp[POOL_NUM_GAMES - 1][t] > maxScore) {
      maxScore = dp[POOL_NUM_GAMES - 1][t];
    }
  }
  return maxScore;
}
POOLDEF uint32_t pool_josh_p_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game) {
  UNUSED(loser);
  if (bracket->winners[game] != winner) return 0;
  return poolConfiguration.roundScores[round] * poolTeams[winner - 1].seed;
}
POOLDEF uint32_t pool_upset_scorer(PoolBracket *bracket, uint8_t winner, uint8_t loser, uint8_t round, uint8_t game) {
  UNUSED(loser);
  if (bracket->winners[game] != winner) return 0;
  return poolConfiguration.roundScores[round] + poolTeams[winner - 1].seed;
}
POOLDEF PoolScorerFunction pool_get_scorer_function(PoolScorerType scorerType) {
  switch(scorerType) {
    case PoolScorerBasic:
      return &pool_basic_scorer;
      break;
    case PoolScorerUpset:
      return &pool_upset_scorer;
      break;
    case PoolScorerJoshP:
      return &pool_josh_p_scorer;
      break;
    case PoolScorerSeedDiff:
      return &pool_seed_diff_scorer;
      break;
    case PoolScorerRelaxedSeedDiff:
      return &pool_relaxed_seed_diff_scorer;
      break;
    case PoolScorerUpsetMultiplier:
      return &pool_upset_multiplier_scorer;
      break;
    default:
      fprintf(stderr, "Error, unknown PoolScorerType: %d\n", scorerType);
      exit(1);
  }
}

static inline uint32_t pool_call_scorer(PoolScorerType type, PoolBracket *bracket,
    uint8_t winner, uint8_t loser, uint8_t round, uint8_t game) {
  switch (type) {
    case PoolScorerBasic: return pool_basic_scorer(bracket, winner, loser, round, game);
    case PoolScorerUpset: return pool_upset_scorer(bracket, winner, loser, round, game);
    case PoolScorerJoshP: return pool_josh_p_scorer(bracket, winner, loser, round, game);
    case PoolScorerSeedDiff: return pool_seed_diff_scorer(bracket, winner, loser, round, game);
    case PoolScorerRelaxedSeedDiff: return pool_relaxed_seed_diff_scorer(bracket, winner, loser, round, game);
    case PoolScorerUpsetMultiplier: return pool_upset_multiplier_scorer(bracket, winner, loser, round, game);
    default: return 0;
  }
}

POOLDEF uint8_t pool_round_of_game(uint8_t gameNum) {
  return poolGamesRound[gameNum];
}

POOLDEF void pool_bracket_score(PoolBracket *bracket, PoolBracket *results) {
  bracket->score = 0;
  bracket->roundScores[0] = 0;
  bracket->wins = 0;
  uint8_t round = 0;
  for (uint8_t g = 0; g < POOL_NUM_GAMES; g++) {
    if (g >= poolGamesInRound[round]) {
      round++;
      bracket->roundScores[round] = 0;
    }
    uint8_t resultsWinner = results->winners[g];
    if (resultsWinner != 0 && bracket->winners[g] == resultsWinner) {
      uint8_t loser = pool_loser_of_game(g, results);
      uint32_t gameScore = (*poolConfiguration.poolScorer)(bracket, resultsWinner, loser, round, g);
      bracket->score += gameScore;
      bracket->roundScores[round] += gameScore;
      bracket->wins += 1;
    }
  }
  if (pool_games_played() < POOL_NUM_GAMES) {
    bracket->maxScore = pool_dp_max_score(bracket, results);
  } else {
    bracket->maxScore = bracket->score;
  }
  if (results->tieBreak > 0) {
    bracket->tieBreakDiff = abs(results->tieBreak - bracket->tieBreak);
  }
}

POOLDEF void pool_defaults(void) {
  // Set defaults
  strcpy(poolConfiguration.poolName, "NCAA Tournament");
  poolConfiguration.roundScores[0] = 1;
  poolConfiguration.roundScores[1] = 2;
  poolConfiguration.roundScores[2] = 4;
  poolConfiguration.roundScores[3] = 8;
  poolConfiguration.roundScores[4] = 16;
  poolConfiguration.roundScores[5] = 32;
  poolConfiguration.scorerType = PoolScorerBasic;
  poolConfiguration.poolScorer = pool_get_scorer_function(poolConfiguration.scorerType);
  strcpy(poolConfiguration.regionNames[0], "Region 1");
  strcpy(poolConfiguration.regionNames[1], "Region 2");
  strcpy(poolConfiguration.regionNames[2], "Region 3");
  strcpy(poolConfiguration.regionNames[3], "Region 4");
}

POOLDEF void pool_initialize(const char *dirPath) {
  DIR *dfd;

  if ((dfd = opendir(dirPath)) == NULL) {
    fprintf(stderr, "Can't open directory %s: %s\n",
        dirPath, strerror(errno));
    exit(1);
  }

  strncpy(poolConfiguration.dirPath, dirPath, 1023);

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
  pool_read_entry_to_bracket(filePath, "Tourney", 7, &poolTournamentBracket, true);

  closedir(dfd);
}


POOLDEF void pool_print_entry(PoolBracket *bracket) {
  if (bracket != &poolTournamentBracket) {
    if (poolTournamentBracket.tieBreak > 0) {
      printf("%s Score: %d Max: %d Tie Break Diff: %d\n", bracket->name, bracket->score, bracket->maxScore, bracket->tieBreakDiff);
    } else {
      printf("%s Score: %d Max: %d\n", bracket->name, bracket->score, bracket->maxScore);
    }
  } else {
    printf("Tournament Results\n");
  }
  printf("          ");
  for (size_t g = 0; g < 32; g++) {
    uint8_t team1, team2 = 0;
    pool_teams_of_game(g, 0, bracket, &team1, &team2);
    if (g > 0) { printf(" "); }
    uint8_t pw = poolTournamentBracket.winners[g];
    printf(STRIKE(team1, pw, "%3s"), POOL_TEAM_SHORT_NAME(team1));
  }
  printf("\n");
  printf("          ");
  for (size_t g = 0; g < 32; g++) {
    uint8_t team1, team2 = 0;
    pool_teams_of_game(g, 0, bracket, &team1, &team2);
    if (g > 0) { printf(" "); }
    uint8_t pw = poolTournamentBracket.winners[g];
    printf(STRIKE(team2, pw, "%3s"), POOL_TEAM_SHORT_NAME(team2));
  }
  printf("\n");
  printf(" Round 1: ");
  for (size_t g = 0; g < 32; g++) {
    uint8_t w = bracket->winners[g];
    uint8_t pw = poolTournamentBracket.winners[g];
    if (g > 0) { printf(" "); }
    printf(STRIKE(w, pw, "%3s"), POOL_TEAM_SHORT_NAME(w));
  }
  printf("\n");
  printf("Sweet 16: ");
  for (size_t g = 0; g < 16; g++) {
    uint8_t w = bracket->winners[g + 32];
    uint8_t pw = poolTournamentBracket.winners[g + 32];
    printf("  ");
    if (g > 0) { printf(" "); }
    printf(STRIKE(w, pw, "%3s"), POOL_TEAM_SHORT_NAME(w));
    printf("  ");
  }
  printf("\n");
  printf(" Elite 8: ");
  for (size_t g = 0; g < 8; g++) {
    uint8_t w = bracket->winners[g + 48];
    uint8_t pw = poolTournamentBracket.winners[g + 48];
    printf("      ");
    if (g > 0) { printf(" "); }
    printf(STRIKE(w, pw, "%3s"), POOL_TEAM_SHORT_NAME(w));
    printf("      ");
  }
  printf("\n");
  printf(" Final 4: ");
  for (size_t g = 0; g < 4; g++) {
    uint8_t w = bracket->winners[g + 56];
    uint8_t pw = poolTournamentBracket.winners[g + 56];
    printf("              ");
    if (g > 0) { printf(" "); }
    printf(STRIKE(w, pw, "%3s"), POOL_TEAM_SHORT_NAME(w));
    printf("              ");
  }
  printf("\n");
  printf(" Final 2: ");
  for (size_t g = 0; g < 2; g++) {
    uint8_t w = bracket->winners[g + 60];
    uint8_t pw = poolTournamentBracket.winners[g + 60];
    printf("                              ");
    if (g > 0) { printf(" "); }
    printf(STRIKE(w, pw, "%3s"), POOL_TEAM_SHORT_NAME(w));
    printf("                             ");
  }
  printf("\n");
  uint8_t w = bracket->winners[62];
  uint8_t pw = poolTournamentBracket.winners[62];
  printf("Champion: ");
  printf(STRIKE(w, pw, "%s"), POOL_TEAM_NAME(w));
  printf(" Tie Breaker: %d", bracket->tieBreak);
  if (poolTournamentBracket.tieBreak > 0) {
    printf(" Diff: %d\n", bracket->tieBreakDiff);
  } else {
    printf("\n");
  }
  printf("\n");
}

POOLDEF int pool_score_cmpfunc (const void * a, const void * b) {
  PoolBracket *bracket_b = (PoolBracket *) b;
  PoolBracket *bracket_a = (PoolBracket *) a;
  int diff = bracket_b->score - bracket_a->score;
  if (diff == 0) {
    if (poolTournamentBracket.tieBreak > 0) {
      diff = bracket_a->tieBreakDiff - bracket_b->tieBreakDiff;
    }
  }
  if (diff == 0) {
    diff = bracket_b->maxScore - bracket_a->maxScore;
  }
  return diff;
}

POOLDEF int pool_champ_counts_cmpfunc (const void * a, const void * b) {
  // Handle 64 bit unsigned ints ... with int return value
  int64_t diff = *((uint64_t *)b) - *((uint64_t *)a);
  if (diff < 0) {
    return -1;
  } else if (diff > 0) {
    return 1;
  }
  return 0;
}

POOLDEF int pool_stats_possible_score_cmpfunc (const void * a, const void * b) {
  return ( ((PoolScoreStats*)b)->possibleScore - ((PoolScoreStats*)a)->possibleScore );
}

POOLDEF int pool_stats_times_won_cmpfunc (const void * a, const void * b) {
  PoolStats *aStats = (PoolStats *) a;
  PoolStats *bStats = (PoolStats *) b;
  int cmp = ( aStats->minRank - bStats->minRank );
  if (cmp == 0) {
    if (bStats->timesWon > aStats->timesWon) {
      cmp = 1;
    } else if (bStats->timesWon < aStats->timesWon) {
      cmp = -1;
    }
  }
  if (cmp == 0) {
    if (bStats->timesTied > aStats->timesTied) {
      cmp = 1;
    } else if (bStats->timesTied < aStats->timesTied) {
      cmp = -1;
    }
  }
  if (cmp == 0) {
    cmp = bStats->maxScore - aStats->maxScore;
  }
  return cmp;
}


POOLDEF void pool_team_report(void) {
  const char *reg_top = "┌────────────────────────────────────────────────────────────────────┐";
  const char *col_top = "├────┬──────────────────────────────────┬───────┬──────┬─────────────┤";
  const char *mid     = "├────┼──────────────────────────────────┼───────┼──────┼─────────────┤";
  const char *bot     = "└────┴──────────────────────────────────┴───────┴──────┴─────────────┘";
  printf("%s: Team Report\n", poolConfiguration.poolName);
  for (int r = 0; r < 4; r++) {
    if (r > 0) printf("\n");
    int nameLen = (int)strlen(poolConfiguration.regionNames[r]);
    int left = (68 - nameLen) / 2;
    int right = 68 - nameLen - left;
    printf("%s\n", reg_top);
    printf("│%*s%s%*s│\n", left, "", poolConfiguration.regionNames[r], right, "");
    printf("%s\n", col_top);
    printf("│ No │               Name               │ Short │ Seed │ Eliminated? │\n");
    printf("%s\n", mid);
    for (int i = r * 16; i < (r + 1) * 16; i++) {
      printf("│ %2d │ %-32s │  %3s  │ %4d │     %-3s     │\n",
          i + 1,
          poolTeams[i].name,
          poolTeams[i].shortName,
          poolTeams[i].seed,
          poolTeams[i].eliminated ? "Yes" : "No"
          );
    }
    printf("%s\n", bot);
  }
}

int power_of_two(int num) {
  int ans = 0;
  if (num == 0)
    return 0;
  while (num != 1) {
    if (num % 2 != 0)
      return 0;
    ans++;
    num = num / 2;
  }
  return ans;
}

POOLDEF void pool_advance_bracket_for_batch(PoolBracket *possibleBracket,
             uint8_t gamesLeft[], int *gamesLeftCount, int batch, int numBatches) {
  if (numBatches <= 1) {
    return;
  }
  int log2Batches = power_of_two(numBatches);
  if (log2Batches > *gamesLeftCount) {
    fprintf(stderr, "[ERROR] There are more batches log2(%d) = %d needed for the games left %d\n",
            numBatches, log2Batches, *gamesLeftCount);
    exit(1);
  }
  int gamesToDecide = LOG2(numBatches);
  for (int g = 0; g < gamesToDecide; g++) {
    uint8_t round = pool_round_of_game(gamesLeft[g]);
    uint8_t team1, team2 = 0;
    pool_teams_of_game(gamesLeft[g], round, possibleBracket, &team1, &team2);
    uint8_t winner = ( batch & (1 << g) ) == 0 ? team1 : team2;
    uint8_t loser = ( batch & (1 << g) ) == 0 ? team2 : team1;
    possibleBracket->winners[gamesLeft[g]] = winner;
    poolTeams[loser-1].eliminated = true;
    /*
    printf("Advancing bracket for game %d of %d : %d, choosing winner %s-%s => %s\n",
           g, gamesToDecide, gamesLeft[g],
           POOL_TEAM_SHORT_NAME(team1),
           POOL_TEAM_SHORT_NAME(team2),
           POOL_TEAM_SHORT_NAME(possibleBracket->winners[gamesLeft[g]]));
    */
  }
  memmove(gamesLeft,
          gamesLeft + gamesToDecide,
          sizeof(uint8_t) * (*gamesLeftCount - gamesToDecide));
  *gamesLeftCount -= gamesToDecide;
}

POOLDEF void pool_possibilities_dfs(
    uint8_t gamesLeft[], int gamesLeftCount, uint8_t game,
    PoolBracket *possibleBracket,
    PoolStats stats[], uint32_t numBrackets,
    PoolProgress *prog,
    PoolBracket possibleBrackets[],
    uint8_t *possibleBracketCount,
    uint16_t freq[], uint32_t maxFreqScore,
    PoolScorerType scorerType) {
  if (game >= gamesLeftCount) {
    pool_inc_progress(prog);
    if (possibleBrackets && possibleBracketCount) {
      memcpy(&possibleBrackets[*possibleBracketCount], possibleBracket, sizeof(PoolBracket));
      *possibleBracketCount += 1;
    }

    // Find the actual max score in the histogram
    while (maxFreqScore > 0 && freq[maxFreqScore] == 0) maxFreqScore--;
    uint32_t champScore = maxFreqScore;
    uint16_t champCount = freq[champScore];
    uint8_t champ = possibleBracket->winners[POOL_NUM_GAMES - 1] - 1;

    // Build rank lookup: rankOfScore[s] = 1 + count of brackets with score > s
    uint16_t rankOfScore[POOL_FREQ_SIZE];
    uint32_t cumulAbove = 0;
    for (int s = (int)champScore; s >= 0; s--) {
      rankOfScore[s] = (uint16_t)(cumulAbove + 1);
      cumulAbove += freq[s];
    }

    // Update stats for all brackets
    for (uint32_t i = 0; i < numBrackets; i++) {
      PoolStats *stat = &stats[i];
      uint16_t realRank = rankOfScore[stat->possibleScore];
      if (stat->possibleScore > stat->maxScore) {
        stat->maxScore = stat->possibleScore;
      }
      if (stat->minRank > realRank) {
        stat->minRank = realRank;
      }
      if (stat->maxRank < realRank) {
        stat->maxRank = realRank;
      }
      if (stat->possibleScore == champScore) {
        stat->champCounts[champ] += 1;
        if (champCount == 1) {
          stat->timesWon += 1;
        } else {
          stat->timesTied += 1;
        }
      }
    }
    return;
  }
  uint8_t gameNum = gamesLeft[game];
  uint8_t round = pool_round_of_game(gameNum);
  uint8_t team1 = 0;
  uint8_t team2 = 0;
  pool_teams_of_game(gameNum, round, possibleBracket, &team1, &team2);
  assert(team1 != 0);
  assert(team2 != 0);
  uint8_t teams[2] = { team1, team2 };
  for (size_t t = 0; t < 2; t++) {
    uint32_t bracketGameScores[numBrackets];
    possibleBracket->winners[gameNum] = teams[t];
    uint8_t otherTeam = teams[1-t];
    poolTeams[otherTeam-1].eliminated = true;
    uint32_t newMaxFreqScore = maxFreqScore;
    for (size_t i = 0; i < numBrackets; i++) {
      bracketGameScores[i] = pool_call_scorer(scorerType, stats[i].bracket, teams[t], otherTeam, round, gameNum);
      freq[stats[i].possibleScore]--;
      stats[i].possibleScore += bracketGameScores[i];
      freq[stats[i].possibleScore]++;
      if (stats[i].possibleScore > newMaxFreqScore) {
        newMaxFreqScore = stats[i].possibleScore;
      }
    }

    // RECURSE
    pool_possibilities_dfs(
      gamesLeft, gamesLeftCount, game + 1,
      possibleBracket,
      stats, poolBracketsCount,
      prog,
      possibleBrackets,
      possibleBracketCount,
      freq, newMaxFreqScore,
      scorerType);

    for (size_t i = 0; i < numBrackets; i++) {
      freq[stats[i].possibleScore]--;
      stats[i].possibleScore -= bracketGameScores[i];
      freq[stats[i].possibleScore]++;
      bracketGameScores[i] = 0;
    }
    possibleBracket->winners[gameNum] = 0;
    poolTeams[otherTeam-1].eliminated = false;
  }
}

POOLDEF PoolReportFormat pool_str_to_format(const char *fmtStr) {
  if (strcasecmp(fmtStr, "text") == 0) {
    return PoolFormatText;
  } else if (strcasecmp(fmtStr, "json") == 0) {
    return PoolFormatJson;
  } else if (strcasecmp(fmtStr, "bin") == 0) {
    return PoolFormatBin;
  } else {
    fprintf(stderr,
        "ERROR: Could not determine report format: %s\n",
        fmtStr);
  }
  return PoolFormatInvalid;
}

POOLDEF void pool_restore_stats_from_files(PoolStats stats[], uint32_t bracketCount) {
  struct dirent *dp;
  DIR *dfd;

  if ((dfd = opendir(poolConfiguration.dirPath)) == NULL) {
    fprintf(stderr, "Can't open directory %s: %s\n",
        poolConfiguration.dirPath, strerror(errno));
    exit(1);
  }

  char entryFilePath[2048];

  while ((dp = readdir(dfd)) != NULL) {
    struct stat stbuf;
    sprintf(entryFilePath, "%s/%s", poolConfiguration.dirPath, dp->d_name);
    if ( stat(entryFilePath, &stbuf) == -1 ) {
      // .. do nothing?
      continue;
    }
    if ( S_ISDIR(stbuf.st_mode) ) {
      // skip directories
      continue;
    }
    if (strncmp(dp->d_name, "poss_", 5) != 0) {
      // skip non-bin files
      continue;
    }
    char *ext = strrchr(entryFilePath, '.');
    if (!ext || strncmp(ext+1, "bin", 3) != 0) {
      // skip no extension or wrong extension
      continue;
    }
    FILE *in = fopen(entryFilePath, "rb");
    if (in == NULL) {
      fprintf(stderr, "Could not open possibility report output bin file %s: %s\n",
        entryFilePath, strerror(errno));
      exit(1);
    }
    int batch, numBatches = 0;
    uint64_t possibleOutcomes = 0;
    uint32_t storedBracketCount = 0;
    if (fread(&batch, sizeof(batch), 1, in) != 1) {
      fprintf(stderr, "ERROR: Could not read batch from %s\n",
              entryFilePath);
      exit(1);
    }
    if (fread(&numBatches, sizeof(numBatches), 1, in) != 1) {
      fprintf(stderr, "ERROR: Could not read numBatches from %s\n",
              entryFilePath);
      exit(1);
    }
    if (fread(&possibleOutcomes, sizeof(possibleOutcomes), 1, in) != 1) {
      fprintf(stderr, "ERROR: Could not read possibleOutcomes from %s\n",
              entryFilePath);
      exit(1);
    }
    if (fread(&storedBracketCount, sizeof(storedBracketCount), 1, in) != 1) {
      fprintf(stderr, "ERROR: Could not read bracketCount from %s\n",
              entryFilePath);
      exit(1);
    }
    assert(storedBracketCount == bracketCount);
    for (size_t i = 0; i < bracketCount; i++) {
      PoolStats partialStat = {0};
      char bracketName[POOL_BRACKET_NAME_LIMIT];
      size_t len = fread(bracketName, 1, POOL_BRACKET_NAME_LIMIT, in);
      assert(len == POOL_BRACKET_NAME_LIMIT);
      if (fread(&partialStat, sizeof(PoolStats), 1, in) != 1) {
        fprintf(stderr, "ERROR: Could not read PoolStats from %s\n",
            entryFilePath);
        exit(1);
      }
      PoolStats *aggStats = NULL;
      for (size_t b = 0; b < bracketCount; b++) {
        if (strcmp(stats[b].bracket->name, bracketName) == 0) {
          aggStats = &stats[b];
        }
      }
      assert(aggStats);
      // combine partial stats with aggregate aggStats
      aggStats->maxRank = (aggStats->maxRank < partialStat.maxRank ? partialStat.maxRank : aggStats->maxRank);
      aggStats->minRank = (aggStats->minRank > partialStat.minRank ? partialStat.minRank : aggStats->minRank);
      aggStats->maxScore = (aggStats->maxScore < partialStat.maxScore ? partialStat.maxScore : aggStats->maxScore);
      aggStats->timesWon += partialStat.timesWon;
      aggStats->timesTied += partialStat.timesTied;
      for (size_t t = 0; t < POOL_NUM_TEAMS; t++) {
        aggStats->champCounts[t] += partialStat.champCounts[t];
      }
    }
    fclose(in);
  }
}

bool setup_possibilities(PoolStats *stats, PoolReportFormat fmt, bool progress,
                         int batch, int numBatches, bool restore, uint64_t *possibleOutcomes,
                         PoolBracket possibleBrackets[], uint8_t *possibleBracketCount) {
  if (poolBracketsCount == 0) {
    fprintf(stderr, ">>>> There are no entries in this pool. <<<<\n");
    return false;
  }

  // Set up posible bracket
  PoolBracket possibleBracket = {
    .winners = {0},
    .tieBreak = poolTournamentBracket.tieBreak,
    .tieBreakDiff = 0,
    .name = "possible",
    .maxScore = 0,
    .roundScores = {0}
  };
  memcpy(possibleBracket.winners, poolTournamentBracket.winners, sizeof(uint8_t) * POOL_NUM_TEAMS);

  // Set up data for DFS
  uint8_t gamesLeft[POOL_NUM_GAMES];
  int gamesLeftCount = 0;
  for (uint8_t g = 0; g < POOL_NUM_GAMES; g++) {
    if (poolTournamentBracket.winners[g] == 0) {
      gamesLeft[gamesLeftCount] = g;
      gamesLeftCount += 1;
    }
  }

  //if (gamesLeftCount == 0) {
  //  fprintf(stderr, ">>>> There are no games left in this pool. <<<<\n");
  //  return false;
  //}

  // If we are running a batch advance possible bracket accordingly
  pool_advance_bracket_for_batch(&possibleBracket, gamesLeft, &gamesLeftCount, batch, numBatches);

  *possibleOutcomes = 2L << (gamesLeftCount - 1);
  if (fmt == PoolFormatText) {
    printf("There are %d teams and %d games remaining, ",
      gamesLeftCount + 1, gamesLeftCount);
    pool_print_humanized(stdout, *possibleOutcomes, 6);
    printf(" possible outcomes\n");
    printf("%s: Possibilities Report\n", poolConfiguration.poolName);
  }

  for (size_t i = 0; i < poolBracketsCount; i++) {
    stats[i].bracket = &poolBrackets[i];
    pool_bracket_score(&poolBrackets[i], &possibleBracket);
    stats[i].possibleScore = poolBrackets[i].score;
    stats[i].minRank = poolBracketsCount + 1;
  }

  if (!restore) {
    // Initialize score frequency histogram for DFS ranking
    uint16_t freq[POOL_FREQ_SIZE];
    memset(freq, 0, sizeof(freq));
    uint32_t maxFreqScore = 0;
    for (size_t i = 0; i < poolBracketsCount; i++) {
      assert(stats[i].possibleScore < POOL_FREQ_SIZE);
      freq[stats[i].possibleScore]++;
      if (stats[i].possibleScore > maxFreqScore) {
        maxFreqScore = stats[i].possibleScore;
      }
    }

    PoolProgress prog;
    prog.start = clock();
    prog.total = *possibleOutcomes;
    prog.complete = 0;
    prog.nextPercent = 0;

    pool_possibilities_dfs(
      gamesLeft, gamesLeftCount, 0,
      &possibleBracket,
      stats, poolBracketsCount,
      progress ? &prog : NULL,
      possibleBrackets,
      possibleBracketCount,
      freq, maxFreqScore,
      poolConfiguration.scorerType);
  } else {
    pool_restore_stats_from_files(stats, poolBracketsCount);
  }
  return true;
}

POOLDEF void pool_final_four_report(PoolReportFormat fmt) {
  // If num possibilities is > 8, bail
  int gamesLeftCount = 63 - pool_games_played();
  if (gamesLeftCount > 3) {
    fprintf(stderr, "Can only run final four report when 3 or fewer games remain.\n");
    exit(1);
  }

  // Reserve space for stats
  PoolStats stats[POOL_BRACKET_CAPACITY] = {0};
  PoolBracket possibleBrackets[8] = {0};
  uint64_t possibleOutcomes = 0L;
  uint8_t possibleBracketCount = 0;

  if (setup_possibilities(stats, PoolFormatInvalid, false, 0, 1, false, &possibleOutcomes, possibleBrackets, &possibleBracketCount)) {
    const char *title =
      possibleOutcomes > 2 ? "Final Four Report" :
      possibleOutcomes == 2 ? "Championship Report" : "Final Results";

    float totalFeesCollected = poolBracketsCount * poolConfiguration.fee;
    float totalPayout = totalFeesCollected;
    for (uint8_t i = 0; i < POOL_MAX_PAYOUTS; i++) {
      if (poolConfiguration.payouts[i] == -1) {
        totalPayout -= poolConfiguration.fee;
      }
    }

    if (fmt == PoolFormatJson) {
      printf("{");
      printf("\"pool\": {");
      printf("\"name\": \"%s\",", poolConfiguration.poolName);
      printf("\"title\": \"%s\",", title);
      printf("\"feesCollected\": %.2f,", totalFeesCollected);
      printf("\"finalPointsKnown\": %s,", poolTournamentBracket.tieBreak > 0 ? "true" : "false");
      printf("\"finalPoints\": %d", poolTournamentBracket.tieBreak);
      if (possibleOutcomes > 2) {
        PoolTeam r1 = poolTeams[poolTournamentBracket.winners[56] - 1];
        PoolTeam r2 = poolTeams[poolTournamentBracket.winners[57] - 1];
        PoolTeam r3 = poolTeams[poolTournamentBracket.winners[58] - 1];
        PoolTeam r4 = poolTeams[poolTournamentBracket.winners[59] - 1];
        printf(",\"finalFour\": [");
        printf("{\"seed\": %d,\"name\": \"%s\",\"shortName\": \"%s\"},", r1.seed, r1.name, r1.shortName);
        printf("{\"seed\": %d,\"name\": \"%s\",\"shortName\": \"%s\"},", r2.seed, r2.name, r2.shortName);
        printf("{\"seed\": %d,\"name\": \"%s\",\"shortName\": \"%s\"},", r3.seed, r3.name, r3.shortName);
        printf("{\"seed\": %d,\"name\": \"%s\",\"shortName\": \"%s\"}", r4.seed, r4.name, r4.shortName);
        printf("]");
      }
      printf("},");
      printf("\"possibilities\": [");
    } else {
      printf("%s: %s\n", poolConfiguration.poolName, title);
      printf("Fees collected: $%d\n", (int) totalFeesCollected);
      if (possibleOutcomes > 2) {
        PoolTeam r1 = poolTeams[poolTournamentBracket.winners[56] - 1];
        PoolTeam r2 = poolTeams[poolTournamentBracket.winners[57] - 1];
        PoolTeam r3 = poolTeams[poolTournamentBracket.winners[58] - 1];
        PoolTeam r4 = poolTeams[poolTournamentBracket.winners[59] - 1];
        printf("(%d) %s vs. (%d) %s AND (%d) %s vs. (%d) %s\n",
             r1.seed, r1.name, r2.seed, r2.name,
             r3.seed, r3.name, r4.seed, r4.name);
      }
    }

    for (uint8_t i = 0; i < possibleBracketCount; i++) {
      PoolBracket b = possibleBrackets[i];
      PoolTeam ff1 = poolTeams[b.winners[60]-1];
      PoolTeam ff2 = poolTeams[b.winners[61]-1];
      PoolTeam champ = poolTeams[b.winners[62]-1];

      // Calculate score of all brackets based on this possible outcome and then sort them
      for (uint32_t j = 0; j < poolBracketsCount; j++) {
        pool_bracket_score(&poolBrackets[j], &b);
      }
      qsort(poolBrackets, poolBracketsCount, sizeof(PoolBracket), pool_score_cmpfunc);
      uint8_t ranks[poolBracketsCount];
      float payouts[poolBracketsCount];
      memset(ranks, 0, poolBracketsCount * sizeof(uint8_t));
      memset(payouts, 0, poolBracketsCount * sizeof(float));

      // Process top 3 payouts (payouts[0..POOL_MAX_PAYOUTS-2])
      uint8_t numTopRanks = 0;
      while (numTopRanks < POOL_MAX_PAYOUTS - 1 && numTopRanks < poolBracketsCount) {
        PoolBracket entry = poolBrackets[numTopRanks];
        float payout = poolConfiguration.payouts[numTopRanks] == -1 ? poolConfiguration.fee : totalPayout * poolConfiguration.payouts[numTopRanks] / 100.0;
        // Look ahead for ties
        uint8_t numTies = 1;
        float tiedPayoutPool = payout;
        for (uint8_t j = numTopRanks+1; j < poolBracketsCount; j++) {
          if (poolBrackets[j].score == entry.score) {
            if (poolBrackets[j].tieBreakDiff == entry.tieBreakDiff) {
              numTies++;
              if (j < POOL_MAX_PAYOUTS - 1) {
                tiedPayoutPool += poolConfiguration.payouts[j] == -1 ? poolConfiguration.fee : totalPayout * poolConfiguration.payouts[j] / 100.0;
              }
            }
          }
        }
        for (uint8_t j = 0; j < numTies; j++) {
          ranks[numTopRanks+j] = numTopRanks + 1;
          payouts[numTopRanks+j] = tiedPayoutPool / numTies;
        }
        numTopRanks += numTies;
      }

      // Process last place payout (payouts[POOL_MAX_PAYOUTS-1])
      uint8_t lastStart = poolBracketsCount; // sentinel: no last place display
      if (poolConfiguration.payouts[POOL_MAX_PAYOUTS - 1] != 0 && poolBracketsCount > numTopRanks) {
        float lastPayout = poolConfiguration.payouts[POOL_MAX_PAYOUTS - 1] == -1 ? poolConfiguration.fee : totalPayout * poolConfiguration.payouts[POOL_MAX_PAYOUTS - 1] / 100.0;
        PoolBracket lastEntry = poolBrackets[poolBracketsCount - 1];
        uint8_t numLastTies = 1;
        for (int j = (int)poolBracketsCount - 2; j >= (int)numTopRanks; j--) {
          if (poolBrackets[j].score == lastEntry.score && poolBrackets[j].tieBreakDiff == lastEntry.tieBreakDiff) {
            numLastTies++;
          } else {
            break;
          }
        }
        lastStart = poolBracketsCount - numLastTies;
        uint8_t lastRank = lastStart + 1;
        float splitPayout = lastPayout / numLastTies;
        for (uint8_t j = lastStart; j < poolBracketsCount; j++) {
          ranks[j] = lastRank;
          payouts[j] = splitPayout;
        }
      }

      if (fmt == PoolFormatJson) {
        if (i > 0) { printf(","); }
        printf("{");
        printf("\"finalist1\": {\"seed\": %d,\"name\": \"%s\",\"shortName\": \"%s\"},", ff1.seed, ff1.name, ff1.shortName);
        printf("\"finalist2\": {\"seed\": %d,\"name\": \"%s\",\"shortName\": \"%s\"},", ff2.seed, ff2.name, ff2.shortName);
        printf("\"champion\": {\"seed\": %d,\"name\": \"%s\",\"shortName\": \"%s\"},", champ.seed, champ.name, champ.shortName);
        printf("\"entries\": [");
        for (uint32_t j = 0; j < poolBracketsCount; j++) {
          PoolBracket entry = poolBrackets[j];
          uint8_t rank = ranks[j] > 0 ? ranks[j] : j + 1;
          bool isLast = j >= lastStart && lastStart < poolBracketsCount;
          if (j > 0) { printf(","); }
          printf("{");
          printf("\"rank\": %d,", rank);
          printf("\"isLast\": %s,", isLast ? "true" : "false");
          printf("\"score\": %d,", entry.score);
          printf("\"tieBreak\": %d,", entry.tieBreak);
          printf("\"tieBreakDiff\": %d,", entry.tieBreakDiff);
          printf("\"payout\": %.2f,", payouts[j]);
          printf("\"name\": \"%s\"", entry.name);
          printf("}");
        }
        printf("]}");
      } else {
        if (possibleBracketCount > 1) {
          printf("Possibility %d: ", i+1);
        } else {
          printf("Final: ");
        }
        printf("(%d) %s vs (%d) %s Champ: (%d) %s\n", ff1.seed, ff1.name,
               ff2.seed, ff2.name, champ.seed, champ.name);
        if (poolTournamentBracket.tieBreak > 0) {
          printf("Total Points Scored in Final: %d\n", poolTournamentBracket.tieBreak);
        }

        const char *ff_top  = "┌──────┬───────┬────────────┬─────────┬──────────────────────┐";
        const char *ff_mid  = "├──────┼───────┼─────┼──────┼─────────┼──────────────────────┤";
        const char *ff_skip = "├╌╌╌╌╌╌┼╌╌╌╌╌╌╌┼╌╌╌╌╌┼╌╌╌╌╌╌┼╌╌╌╌╌╌╌╌╌┼╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌╌┤";
        const char *ff_bot  = "└──────┴───────┴─────┴──────┴─────────┴──────────────────────┘";
        printf("%s\n", ff_top);
        printf("│      │       │  Tie Break │         │                      │\n");
        printf("│ Rank │ Score │ Pts │ Diff │  Payout │ %-20s │\n", "Name");
        printf("%s\n", ff_mid);
        for (uint8_t j = 0; j < numTopRanks; j++) {
          PoolBracket entry = poolBrackets[j];
          char tieBreakDiff[10] = {0};
          if (entry.tieBreakDiff > 0) {
            sprintf(tieBreakDiff, "%3d", entry.tieBreakDiff);
          } else {
            sprintf(tieBreakDiff, "%s ", "?");
          }
          printf("│ %4d │ %5d │ %3d │ %4s │ $%6.2f │ %-20.20s │\n", ranks[j], entry.score, entry.tieBreak, tieBreakDiff, payouts[j], entry.name);
        }
        if (lastStart < poolBracketsCount) {
          uint8_t displayStart = lastStart > numTopRanks ? lastStart : numTopRanks;
          if (displayStart > numTopRanks) {
            printf("%s\n", ff_skip);
          }
          for (uint8_t j = displayStart; j < poolBracketsCount; j++) {
            PoolBracket entry = poolBrackets[j];
            char tieBreakDiff[10] = {0};
            if (entry.tieBreakDiff > 0) {
              sprintf(tieBreakDiff, "%3d", entry.tieBreakDiff);
            } else {
              sprintf(tieBreakDiff, "%s ", "?");
            }
            printf("│ %4s │ %5d │ %3d │ %4s │ $%6.2f │ %-20.20s │\n", "LAST", entry.score, entry.tieBreak, tieBreakDiff, payouts[j], entry.name);
          }
        }
        printf("%s\n\n", ff_bot);
      }
    }

    if (fmt == PoolFormatJson) {
      printf("]}\n");
    }
  }
}


POOLDEF void pool_possibilities_report(PoolReportFormat fmt, bool progress, int batch, int numBatches, bool restore) {
  // Reserve buffer for writing bin output files
  char filePath[2048];

  // Reserve space for stats
  PoolStats stats[POOL_BRACKET_CAPACITY] = {0};

  uint64_t possibleOutcomes = 0L;

  if (setup_possibilities(stats, fmt, progress, batch, numBatches, restore, &possibleOutcomes, NULL, NULL)) {
    qsort(stats, poolBracketsCount, sizeof(PoolStats), pool_stats_times_won_cmpfunc);

    switch (fmt) {
      case PoolFormatText: {
        const char *sep_top = "┌──────────────────────┬──────┬──────┬───────┬───────┬────────┬────────┬────────┬──────────────────────┐";
        const char *sep_mid = "├──────────────────────┼──────┼──────┼───────┼───────┼────────┼────────┼────────┼──────────────────────┤";
        const char *sep_bot = "└──────────────────────┴──────┴──────┴───────┴───────┴────────┴────────┴────────┴──────────────────────┘";
        printf("%s\n", sep_top);
        printf("│                      │  Min │  Max │  Curr │  Max  │   Win  │  Times │  Times │                      │\n");
        printf("│ %-20s │ Rank │ Rank │ Score │ Score │ Chance │   Won  │   Tied │ %-20s │\n",
            "Name", "Top Champs");
        printf("%s\n", sep_mid);
        for (size_t i = 0; i < poolBracketsCount; i++) {
          PoolStats *stat = &stats[i];
          float winChance = possibleOutcomes == 0 ? (stat->timesWon > 0 ? 100.0f : 0.0f)
                                                  : (double) stat->timesWon / (double) possibleOutcomes * 100.0;
          char champsStr[32] = "";
          if (stat->timesWon > 0 || stat->timesTied > 0) {
            PoolTeamWins top5[5] = {0};
            for (size_t t = 0; t < POOL_NUM_TEAMS; t++) {
              if (stat->champCounts[t] > 0) {
                for(size_t j = 0; j < 5; j++) {
                  if (stat->champCounts[t] > top5[j].count) {
                    for(size_t k = 4; k > j; k--) {
                      top5[k].team = top5[k-1].team;
                      top5[k].count = top5[k-1].count;
                    }
                    top5[j].team = t + 1;
                    top5[j].count = stat->champCounts[t];
                    break;
                  }
                }
              }
            }
            char *cp = champsStr;
            for (size_t w = 0; w < 5; w++) {
              if (top5[w].team != 0) {
                if (w > 0) cp += sprintf(cp, ",");
                cp += sprintf(cp, "%s", POOL_TEAM_SHORT_NAME(top5[w].team));
              }
            }
          }
          printf("│ %-20.20s │ %4d │ %4d │ %5d │ %5d │ %6.2f │ ",
              stat->bracket->name,
              stat->minRank, stat->maxRank, stat->bracket->score,
              stat->maxScore, winChance);
          pool_print_humanized(stdout, stat->timesWon, 5);
          printf(" │ ");
          pool_print_humanized(stdout, stat->timesTied, 5);
          printf(" │ %-20.20s │\n", champsStr);
        }
        printf("%s\n", sep_bot);
        break;
      }
      case PoolFormatJson:
        printf("{");
        printf("\"pool\": {");
        printf("\"name\": \"%s\",", poolConfiguration.poolName);
        printf("\"outcomes\": %" PRIu64 ",", possibleOutcomes);
        printf("\"batch\": %d,", batch);
        printf("\"numBatches\": %d", numBatches);
        printf("},");
        printf("\"entries\": [");
        for (size_t i = 0; i < poolBracketsCount; i++) {
          PoolStats *stat = &stats[i];
          float winChance = possibleOutcomes == 0 ? (stat->timesWon > 0 ? 100.0f : 0.0f)
                                                  : (float) stat->timesWon / (float) possibleOutcomes * 100.0;
          if (i > 0) { printf(","); }
          printf("{");
          printf("\"name\": \"%s\",", stat->bracket->name);
          printf("\"minRank\": %d,", stat->minRank);
          printf("\"maxRank\": %d,", stat->maxRank);
          printf("\"currentScore\": %d,", stat->bracket->score);
          printf("\"maxScore\": %d,", stat->maxScore);
          printf("\"winChance\": %.6f,", winChance);
          printf("\"timesWon\": %" PRIu64 ",", stat->timesWon);
          printf("\"timesTied\": %" PRIu64 ",", stat->timesTied);
          printf("\"champs\": [");
          bool first = true;
          for (size_t t = 0; t < POOL_NUM_TEAMS; t++) {
            if (stat->champCounts[t] > 0) {
              if (!first) { printf(","); }
              printf("{");
              printf("\"team\": {");
              printf("\"number\": %ld,", t + 1);
              printf("\"shortName\": \"%s\"", POOL_TEAM_SHORT_NAME(t + 1));
              printf("},");
              printf("\"timesWon\": %" PRIu64, stat->champCounts[t]);
              printf("}");
              first = false;
            }
          }
          printf("]");
          printf("}");
        }
        printf("]");
        printf("}\n");
        break;
      case PoolFormatBin:
        sprintf(filePath, "%s/poss_%d_of_%d.bin", poolConfiguration.dirPath, batch, numBatches);
        FILE * out = fopen(filePath, "wb");
        if (out == NULL) {
          fprintf(stderr, "Could not open possibility report output bin file %s: %s\n",
            filePath, strerror(errno));
          exit(1);
        }
        fwrite(&batch, sizeof(batch), 1, out);
        fwrite(&numBatches, sizeof(numBatches), 1, out);
        fwrite(&possibleOutcomes, sizeof(possibleOutcomes), 1, out);
        fwrite(&poolBracketsCount, sizeof(poolBracketsCount), 1, out);
        for (size_t i = 0; i < poolBracketsCount; i++) {
          PoolStats *stat = &stats[i];
          fwrite(stat->bracket->name, 1, POOL_BRACKET_NAME_LIMIT, out);
          fwrite(stat, sizeof(PoolStats), 1, out);
        }
        fclose(out);
        break;
      case PoolFormatInvalid:
      default:
        fprintf(stderr, "ERROR: Unreachable, unknown or invalid PoolReportFormat\n");
        exit(1);
    }
  }
}

// Stat IDs (matching weights.json keys) and corresponding CSV column headers.
// Index 0 is Seed, which is treated specially during scoring (inverted).
static const char *pool_model_stat_ids[POOL_NUM_MODEL_STATS] = {
  "Seed", "WP", "SS", "PG", "OPG", "FGP", "3PFGP", "FTP", "OR", "DR",
  "ASM", "RP", "ORP", "EFGP", "TSP", "OTSP", "P", "TP", "OTP", "TM",
  "AP", "AT", "FTFGA", "OFTFGA"
};
static const char *pool_model_stat_csv_names[POOL_NUM_MODEL_STATS] = {
  "Seed", "Win %", "SoS", "Pts / Gm", "Opp Pts / Gm", "FG %", "3Pt FG %",
  "Free Throw %", "Offense Rating", "Defense Rating", "Adj. Score Margin",
  "Rebound %", "Off. Rebound %", "Effective FG %", "True Shooting %",
  "Opp. True Shoot %", "Pace", "Turnover %", "Opp. Turnover %",
  "Turnover Margin", "Assist %", "Assists / Turnover", "FT / FGA", "Opp. FT / FGA"
};

// Split a CSV line into fields by replacing commas with NUL terminators.
// Trims trailing CR/LF from the last field. Returns the number of fields.
static int pool_csv_split(char *line, char *fields[], int maxFields) {
  int n = 0;
  char *p = line;
  while (n < maxFields) {
    fields[n++] = p;
    p = strchr(p, ',');
    if (!p) break;
    *p++ = '\0';
  }
  if (n > 0) {
    char *end = fields[n - 1] + strlen(fields[n - 1]) - 1;
    while (end >= fields[n - 1] && (*end == '\n' || *end == '\r')) *end-- = '\0';
  }
  return n;
}

// Load weights.json from the pool directory into poolModelWeights[].
static bool pool_load_model_weights_file(const char *dirPath) {
  char path[2048];
  snprintf(path, sizeof(path), "%s/%s", dirPath, POOL_WEIGHTS_FILE_NAME);
  FILE *f = fopen(path, "r");
  if (!f) { fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno)); return false; }
  char line[256];
  int loaded = 0;
  while (fgets(line, sizeof(line), f)) {
    char key[32];
    double val;
    if (sscanf(line, " \"%31[^\"]\" : %lf", key, &val) == 2) {
      for (int i = 0; i < POOL_NUM_MODEL_STATS; i++) {
        if (strcmp(key, pool_model_stat_ids[i]) == 0) {
          poolModelWeights[i] = (float)val;
          loaded++;
          break;
        }
      }
    }
  }
  fclose(f);
  return loaded > 0;
}

// Load stats.csv from the pool directory into poolTeamModelStats[][].
// The file must begin with front-matter comment lines mapping anglebracket
// region integers to pool region names, e.g.:
//   # region 0: South
//   # region 1: East
//   # region 2: West
//   # region 3: Midwest
// Teams are matched by (region, seed) — no name matching required.
static bool pool_load_model_stats_file(const char *dirPath) {
  char path[2048];
  snprintf(path, sizeof(path), "%s/%s", dirPath, POOL_STATS_FILE_NAME);
  FILE *f = fopen(path, "r");
  if (!f) { fprintf(stderr, "Cannot open %s: %s\n", path, strerror(errno)); return false; }

  int regionMap[4] = {-1, -1, -1, -1};  // anglebracket region int -> pool region index
  char line[4096];
  char headerLine[4096] = {0};

  // Read front matter (# lines) until the CSV header
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#') {
      int regNum;
      char regName[64];
      if (sscanf(line, "# region %d: %63[^\n\r]", &regNum, regName) == 2 &&
          regNum >= 0 && regNum < 4) {
        for (int r = 0; r < 4; r++) {
          if (strcasecmp(poolConfiguration.regionNames[r], regName) == 0) {
            regionMap[regNum] = r;
            break;
          }
        }
      }
    } else {
      strncpy(headerLine, line, sizeof(headerLine) - 1);
      break;
    }
  }

  if (headerLine[0] == '\0') {
    fprintf(stderr, "stats.csv: no CSV header found\n");
    fclose(f); return false;
  }
  for (int r = 0; r < 4; r++) {
    if (regionMap[r] < 0) {
      fprintf(stderr, "stats.csv: anglebracket region %d not mapped to a pool region\n", r);
      fclose(f); return false;
    }
  }

  // Parse CSV header for column indices
  char *fields[64];
  int regionCol = -1, seedCol = -1;
  int statCol[POOL_NUM_MODEL_STATS];
  for (int i = 0; i < POOL_NUM_MODEL_STATS; i++) statCol[i] = -1;

  int numCols = pool_csv_split(headerLine, fields, 64);
  for (int c = 0; c < numCols; c++) {
    if (strcmp(fields[c], "Region") == 0) { regionCol = c; continue; }
    if (strcmp(fields[c], "Seed")   == 0) { seedCol   = c; continue; }
    for (int s = 0; s < POOL_NUM_MODEL_STATS; s++) {
      if (strcmp(fields[c], pool_model_stat_csv_names[s]) == 0) {
        statCol[s] = c; break;
      }
    }
  }
  // Seed is both a match key and a stat; find its stat column via csv name
  if (seedCol >= 0 && statCol[0] < 0) statCol[0] = seedCol;

  if (regionCol < 0 || seedCol < 0) {
    fprintf(stderr, "stats.csv: missing Region or Seed column\n");
    fclose(f); return false;
  }

  // Read data rows and populate poolTeamModelStats by region+seed lookup
  int matched = 0;
  while (fgets(line, sizeof(line), f)) {
    int n = pool_csv_split(line, fields, 64);
    if (n <= regionCol || n <= seedCol) continue;
    int abRegion = atoi(fields[regionCol]);
    int seed     = atoi(fields[seedCol]);
    if (abRegion < 0 || abRegion >= 4 || regionMap[abRegion] < 0) continue;
    int poolRegion = regionMap[abRegion];

    int teamIdx = -1;
    for (int i = poolRegion * 16; i < poolRegion * 16 + 16; i++) {
      if (poolTeams[i].seed == (uint8_t)seed) { teamIdx = i; break; }
    }
    if (teamIdx < 0) continue;

    for (int s = 0; s < POOL_NUM_MODEL_STATS; s++) {
      if (statCol[s] < 0 || statCol[s] >= n) continue;
      poolTeamModelStats[teamIdx][s] = (float)atof(fields[statCol[s]]);
    }
    matched++;
  }

  fclose(f);
  if (matched == 0) fprintf(stderr, "stats.csv: no teams matched\n");
  return matched > 0;
}

POOLDEF bool pool_load_model_data(void) {
  bool weights_ok = pool_load_model_weights_file(poolConfiguration.dirPath);
  bool stats_ok   = pool_load_model_stats_file(poolConfiguration.dirPath);
  poolModelStatsLoaded = weights_ok && stats_ok;
  return poolModelStatsLoaded;
}

// xorshift64 RNG — fast, no external dependencies
static uint64_t pool_rng_next(uint64_t *state) {
  *state ^= *state << 13;
  *state ^= *state >> 7;
  *state ^= *state << 17;
  return *state;
}

// Returns winning team number (1-indexed) using seed-weighted probability.
// P(team1 wins) = s2 / (s1 + s2), biasing toward lower seeds.
static uint8_t pool_mc_pick_winner(uint8_t team1, uint8_t team2, uint64_t *rng) {
  uint8_t s1 = poolTeams[team1 - 1].seed;
  uint8_t s2 = poolTeams[team2 - 1].seed;
  return (pool_rng_next(rng) % (s1 + s2) < s2) ? team1 : team2;
}

// Compute the anglebracket model score for a team (0-indexed teamIdx).
// Seed (stat index 0) is inverted so that lower seeds score higher.
static float pool_mc_model_score(int teamIdx) {
  float score = 0.0f;
  for (int i = 0; i < POOL_NUM_MODEL_STATS; i++) {
    if (poolModelWeights[i] == 0.0f) continue;
    float sv = poolTeamModelStats[teamIdx][i];
    if (i == 0) sv = (16.0f - sv) / 16.0f;  // invert seed
    score += sv * poolModelWeights[i];
  }
  return score;
}

// Pick a winner using model score ratio as win probability.
// P(team1 wins) = score1 / (score1 + score2).
// Falls back to seed-weighted if scores are unavailable.
static uint8_t pool_mc_pick_winner_model(uint8_t team1, uint8_t team2, uint64_t *rng) {
  float s1 = pool_mc_model_score(team1 - 1);
  float s2 = pool_mc_model_score(team2 - 1);
  float total = s1 + s2;
  assert(total > 0.0f);
  uint64_t threshold = (uint64_t)(s1 / total * 1000000.0f);
  return (pool_rng_next(rng) % 1000000 < threshold) ? team1 : team2;
}

POOLDEF void pool_monte_carlo_report(uint64_t numSamples, PoolReportFormat fmt, bool progress, PoolMCSelectionMode selectionMode) {
  if (poolBracketsCount == 0) {
    fprintf(stderr, ">>>> There are no entries in this pool. <<<<\n");
    return;
  }

  // Load model data if needed
  if (selectionMode == PoolMCSelectionModelWeighted && !poolModelStatsLoaded) {
    if (!pool_load_model_data()) {
      fprintf(stderr, "Model data failed to load; cannot run Monte Carlo report\n");
      return;
    }
  }

  // Build possibleBracket from current tournament state
  PoolBracket possibleBracket;
  memset(&possibleBracket, 0, sizeof(possibleBracket));
  memcpy(possibleBracket.winners, poolTournamentBracket.winners, sizeof(uint8_t) * POOL_NUM_TEAMS);
  possibleBracket.tieBreak = poolTournamentBracket.tieBreak;
  strncpy(possibleBracket.name, "possible", POOL_BRACKET_NAME_LIMIT - 1);

  // Build gamesLeft[] — games without a known result yet
  uint8_t gamesLeft[POOL_NUM_GAMES];
  int gamesLeftCount = 0;
  for (uint8_t g = 0; g < POOL_NUM_GAMES; g++) {
    if (poolTournamentBracket.winners[g] == 0) {
      gamesLeft[gamesLeftCount++] = g;
    }
  }

  if (fmt == PoolFormatText) {
    printf("There are %d games remaining, ", gamesLeftCount);
    pool_print_humanized(stdout, numSamples, 6);
    printf(" Monte Carlo simulations\n");
    printf("%s: Monte Carlo Possibilities Report\n", poolConfiguration.poolName);
  }

  // Pre-compute base scores (against current results, no future games)
  uint32_t baseScore[POOL_BRACKET_CAPACITY];
  for (size_t i = 0; i < poolBracketsCount; i++) {
    pool_bracket_score(&poolBrackets[i], &possibleBracket);
    baseScore[i] = poolBrackets[i].score;
  }

  // Initialize stats
  PoolStats stats[POOL_BRACKET_CAPACITY];
  memset(stats, 0, sizeof(stats));
  for (size_t i = 0; i < poolBracketsCount; i++) {
    stats[i].bracket = &poolBrackets[i];
    stats[i].possibleScore = baseScore[i];
    stats[i].minRank = (uint16_t)(poolBracketsCount + 1);
  }

  // Seed RNG from current time; avoid zero state
  uint64_t rng = (uint64_t)time(NULL);
  if (rng == 0) rng = 0xDEADBEEFCAFEULL;
  // Warm up RNG
  for (int w = 0; w < 16; w++) pool_rng_next(&rng);

  // Reusable per-iteration buffers
  uint32_t totalScore[POOL_BRACKET_CAPACITY];
  uint16_t freq[POOL_FREQ_SIZE];
  uint16_t rankOfScore[POOL_FREQ_SIZE];

  clock_t startTime = clock();
  uint64_t progressStep = numSamples / 100;
  if (progressStep == 0) progressStep = 1;

  for (uint64_t sim = 0; sim < numSamples; sim++) {
    // Build simulated bracket: copy current state then fill remaining games
    PoolBracket simBracket;
    memcpy(simBracket.winners, possibleBracket.winners, sizeof(uint8_t) * POOL_NUM_TEAMS);

    for (int gi = 0; gi < gamesLeftCount; gi++) {
      uint8_t gameNum = gamesLeft[gi];
      uint8_t round = pool_round_of_game(gameNum);
      uint8_t team1, team2;
      pool_teams_of_game(gameNum, round, &simBracket, &team1, &team2);
      uint8_t winner;
      if (team2 == 0) {
        winner = team1;
      } else {
        switch (selectionMode) {
          case PoolMCSelectionModelWeighted:
            winner = pool_mc_pick_winner_model(team1, team2, &rng);
            break;
          case PoolMCSelectionSeedWeighted:
          default:
            winner = pool_mc_pick_winner(team1, team2, &rng);
            break;
        }
      }
      simBracket.winners[gameNum] = winner;
    }

    // Score all entries incrementally (base + contribution from simulated games)
    for (size_t i = 0; i < poolBracketsCount; i++) {
      totalScore[i] = baseScore[i];
    }
    for (int gi = 0; gi < gamesLeftCount; gi++) {
      uint8_t gameNum = gamesLeft[gi];
      uint8_t round = pool_round_of_game(gameNum);
      uint8_t winner = simBracket.winners[gameNum];
      uint8_t loser = pool_loser_of_game(gameNum, &simBracket);
      for (size_t i = 0; i < poolBracketsCount; i++) {
        totalScore[i] += pool_call_scorer(poolConfiguration.scorerType,
                                          &poolBrackets[i], winner, loser, round, gameNum);
      }
    }

    // Build score frequency histogram
    memset(freq, 0, sizeof(freq));
    uint32_t maxFreqScore = 0;
    for (size_t i = 0; i < poolBracketsCount; i++) {
      assert(totalScore[i] < POOL_FREQ_SIZE);
      freq[totalScore[i]]++;
      if (totalScore[i] > maxFreqScore) maxFreqScore = totalScore[i];
    }

    // Find champion score (highest score with at least one entry)
    uint32_t champScore = maxFreqScore;
    while (champScore > 0 && freq[champScore] == 0) champScore--;
    uint16_t champCount = freq[champScore];
    uint8_t champ = simBracket.winners[POOL_NUM_GAMES - 1] - 1;  // 0-indexed team

    // Build rank lookup: rankOfScore[s] = 1 + count of entries scoring above s
    uint32_t cumulAbove = 0;
    for (int s = (int)champScore; s >= 0; s--) {
      rankOfScore[s] = (uint16_t)(cumulAbove + 1);
      cumulAbove += freq[s];
    }

    // Update stats for each entry
    for (size_t i = 0; i < poolBracketsCount; i++) {
      PoolStats *stat = &stats[i];
      uint16_t realRank = rankOfScore[totalScore[i]];
      if (totalScore[i] > stat->maxScore) stat->maxScore = totalScore[i];
      if (stat->minRank > realRank) stat->minRank = realRank;
      if (stat->maxRank < realRank) stat->maxRank = realRank;
      if (totalScore[i] == champScore) {
        stat->champCounts[champ]++;
        if (champCount == 1) {
          stat->timesWon++;
        } else {
          stat->timesTied++;
        }
      }
    }

    // Progress display
    if (progress && (sim + 1) % progressStep == 0) {
      double perc = (double)(sim + 1) / (double)numSamples * 100.0;
      clock_t curr = clock();
      double elapsed = (double)(curr - startTime) / CLOCKS_PER_SEC;
      double sps = elapsed > 0 ? (double)(sim + 1) / elapsed : 0.0;
      uint64_t remaining = numSamples - sim - 1;
      uint64_t eta = sps > 0 ? (uint64_t)((double)remaining / sps) : 0;
      fprintf(stderr, "MC SPS: %12.0f %3.0f%% ETA: %02" PRIu64 ":%02" PRIu64 ":%02" PRIu64 "\r",
              sps, perc, eta / 3600, (eta % 3600) / 60, eta % 60);
      fflush(stderr);
    }
  }
  if (progress) {
    fprintf(stderr, "\n");
  }

  // Sort by best win chance (same comparator as poss report)
  qsort(stats, poolBracketsCount, sizeof(PoolStats), pool_stats_times_won_cmpfunc);

  switch (fmt) {
    case PoolFormatText: {
      const char *sep_top = "┌──────────────────────┬──────┬──────┬────────────────┬─────────┬────────┬──────────────────────┐";
      const char *sep_mid = "├──────────────────────┼──────┼──────┼────────────────┼─────────┼────────┼──────────────────────┤";
      const char *sep_bot = "└──────────────────────┴──────┴──────┴────────────────┴─────────┴────────┴──────────────────────┘";
      printf("%s\n", sep_top);
      printf("│                      │  Min │  Max │                │  Tied%%  │  Simul │                      │\n");
      printf("│ %-20s │ Rank │ Rank │      Win%%      │         │   Won  │ %-20s │\n",
             "Name", "Top Champs");
      printf("%s\n", sep_mid);
      for (size_t i = 0; i < poolBracketsCount; i++) {
        PoolStats *stat = &stats[i];
        double p = numSamples > 0 ? (double)stat->timesWon / (double)numSamples : 0.0;
        double tiedPct = numSamples > 0 ? (double)stat->timesTied / (double)numSamples * 100.0 : 0.0;

        // Build Top Champs string (top 5 by champCounts)
        char champsStr[32] = "";
        if (stat->timesWon > 0 || stat->timesTied > 0) {
          PoolTeamWins top5[5] = {0};
          for (size_t t = 0; t < POOL_NUM_TEAMS; t++) {
            if (stat->champCounts[t] > 0) {
              for (size_t j = 0; j < 5; j++) {
                if (stat->champCounts[t] > top5[j].count) {
                  for (size_t k = 4; k > j; k--) {
                    top5[k].team = top5[k-1].team;
                    top5[k].count = top5[k-1].count;
                  }
                  top5[j].team = (uint8_t)(t + 1);
                  top5[j].count = stat->champCounts[t];
                  break;
                }
              }
            }
          }
          char *cp = champsStr;
          for (size_t w = 0; w < 5; w++) {
            if (top5[w].team != 0) {
              if (w > 0) cp += sprintf(cp, ",");
              cp += sprintf(cp, "%s", POOL_TEAM_SHORT_NAME(top5[w].team));
            }
          }
        }

        // Print name, min/max rank
        printf("│ %-20.20s │ %4d │ %4d │ ",
               stat->bracket->name, stat->minRank, stat->maxRank);

        // Win% column: 14 display chars
        if (stat->timesWon == 0) {
          printf("  0.00%%       ");
        } else if (stat->timesWon < 5) {
          printf("< 0.001%%      ");
        } else {
          double margin = 1.96 * sqrt(p * (1.0 - p) / (double)numSamples);
          printf("%6.2f%% \xc2\xb1%4.2f%%", p * 100.0, margin * 100.0);
        }

        // Tied%, Simul Won, Top Champs
        printf(" │ %6.2f%% │ ", tiedPct);
        pool_print_humanized(stdout, stat->timesWon, 5);
        printf(" │ %-20.20s │\n", champsStr);
      }
      printf("%s\n", sep_bot);
      break;
    }
    case PoolFormatJson: {
      printf("{");
      printf("\"pool\": {");
      printf("\"name\": \"%s\",", poolConfiguration.poolName);
      printf("\"numSamples\": %" PRIu64, numSamples);
      printf("},");
      printf("\"entries\": [");
      for (size_t i = 0; i < poolBracketsCount; i++) {
        PoolStats *stat = &stats[i];
        double p = numSamples > 0 ? (double)stat->timesWon / (double)numSamples : 0.0;
        double margin = (stat->timesWon >= 5)
            ? 1.96 * sqrt(p * (1.0 - p) / (double)numSamples)
            : 0.0;
        double tiedPct = numSamples > 0 ? (double)stat->timesTied / (double)numSamples * 100.0 : 0.0;
        if (i > 0) { printf(","); }
        printf("{");
        printf("\"name\": \"%s\",", stat->bracket->name);
        printf("\"minRank\": %d,", stat->minRank);
        printf("\"maxRank\": %d,", stat->maxRank);
        printf("\"winPct\": %.6f,", p * 100.0);
        printf("\"winMargin\": %.6f,", margin * 100.0);
        printf("\"tiedPct\": %.6f,", tiedPct);
        printf("\"timesWon\": %" PRIu64 ",", stat->timesWon);
        printf("\"timesTied\": %" PRIu64 ",", stat->timesTied);
        printf("\"champs\": [");
        bool first = true;
        for (size_t t = 0; t < POOL_NUM_TEAMS; t++) {
          if (stat->champCounts[t] > 0) {
            if (!first) { printf(","); }
            printf("{");
            printf("\"team\": {");
            printf("\"number\": %zu,", t + 1);
            printf("\"shortName\": \"%s\"", POOL_TEAM_SHORT_NAME(t + 1));
            printf("},");
            printf("\"timesWon\": %" PRIu64, stat->champCounts[t]);
            printf("}");
            first = false;
          }
        }
        printf("]");
        printf("}");
      }
      printf("]");
      printf("}\n");
      break;
    }
    default:
      fprintf(stderr, "ERROR: mc command only supports 'text' and 'json' formats\n");
      exit(1);
  }
}

POOLDEF void pool_score_report(PoolReportFormat fmt) {
  if (poolBracketsCount == 0) {
    fprintf(stderr, ">>>> There are no entries in this pool. <<<<\n");
    return;
  }
  for (size_t i = 0; i < poolBracketsCount; i++) {
    PoolBracket *bracket = &poolBrackets[i];
    pool_bracket_score(bracket, &poolTournamentBracket);
  }
  uint8_t rank = 1;
  uint32_t lastScore = 0;
  qsort(poolBrackets, poolBracketsCount, sizeof(PoolBracket), pool_score_cmpfunc);

  if (fmt == PoolFormatJson) {
    int gamesPlayedCount = pool_games_played();
    int gamesLeftCount = 63 - gamesPlayedCount;
    printf("{");
    printf("\"pool\": {");
    printf("\"name\": \"%s\",", poolConfiguration.poolName);
    printf("\"finalPointsKnown\": %s,", poolTournamentBracket.tieBreak > 0 ? "true" : "false");
    printf("\"finalPoints\": %d,", poolTournamentBracket.tieBreak);
    printf("\"gamesPlayed\": %d,", gamesPlayedCount);
    printf("\"gamesRemaining\": %d", gamesLeftCount);
    if (gamesLeftCount > 0) {
      uint64_t possibleOutcomes = 2L << (gamesLeftCount - 1);
      printf(",\"possibleOutcomes\": %" PRIu64, possibleOutcomes);
    }
    if (poolTournamentBracket.winners[62] > 0) {
      printf(",\"champion\": \"%s\"", POOL_TEAM_SHORT_NAME(poolTournamentBracket.winners[62]));
    }
    printf("},");
    printf("\"entries\": [");
    for (size_t i = 0; i < poolBracketsCount; i++) {
      if (i > 0) {
        if (poolBrackets[i].score != lastScore || poolTournamentBracket.tieBreak > 0) {
          rank = i + 1;
          lastScore = poolBrackets[i].score;
        }
        printf(",");
      } else {
        lastScore = poolBrackets[i].score;
      }
      bool champAlive = poolBrackets[i].winners[62] > 0 &&
                        !poolTeams[poolBrackets[i].winners[62] - 1].eliminated;
      printf("{");
      printf("\"rank\": %d,", rank);
      printf("\"name\": \"%s\",", poolBrackets[i].name);
      printf("\"score\": %d,", poolBrackets[i].score);
      printf("\"maxScore\": %d,", poolBrackets[i].maxScore);
      printf("\"correctPicks\": %d,", poolBrackets[i].wins);
      printf("\"champShortName\": \"%s\",", POOL_TEAM_SHORT_NAME(poolBrackets[i].winners[62]));
      printf("\"champAlive\": %s,", champAlive ? "true" : "false");
      printf("\"tieBreak\": %d,", poolBrackets[i].tieBreak);
      printf("\"tieBreakDiff\": %d,", poolBrackets[i].tieBreakDiff);
      printf("\"roundScores\": [%d,%d,%d,%d,%d,%d]",
        poolBrackets[i].roundScores[0], poolBrackets[i].roundScores[1],
        poolBrackets[i].roundScores[2], poolBrackets[i].roundScores[3],
        poolBrackets[i].roundScores[4], poolBrackets[i].roundScores[5]);
      printf("}");
    }
    printf("]}");
    printf("\n");
    return;
  }

  printf("%s: Leaderboard\n", poolConfiguration.poolName);
  if (poolTournamentBracket.winners[62] > 0) {
    printf("Champion: %s Final Points: ", POOL_TEAM_NAME(poolTournamentBracket.winners[62]));
    if (poolTournamentBracket.tieBreak > 0) {
      printf("%d\n", poolTournamentBracket.tieBreak);
    } else {
      printf("UNK\n");
    }
  }
  const char *top = "┌──────┬──────────────────────┬───────┬───────┬─────────┬───────┬───────┬──────┬───────────────────────────────────┐";
  const char *mid = "├──────┼──────────────────────┼───────┼───────┼─────────┼───────┼───────┼──────┼─────┼─────┼─────┼─────┼─────┼─────┤";
  const char *bot = "└──────┴──────────────────────┴───────┴───────┴─────────┴───────┴───────┴──────┴─────┴─────┴─────┴─────┴─────┴─────┘";
  printf("%s\n", top);
  printf("│      │                      │  Curr │  Max  │ Correct │ Champ │  Tie  │      │           Round Scores            │\n");
  printf("│ Rank │ Name                 │ Score │ Score │  Picks  │ Live? │ Break │ Diff │  1  │  2  │  3  │  4  │  5  │  6  │\n");
  printf("%s\n", mid);
  for (size_t i = 0; i < poolBracketsCount; i++) {
    if (i > 0) {
      if (poolBrackets[i].score != lastScore || poolTournamentBracket.tieBreak > 0) {
        rank = i + 1;
        lastScore = poolBrackets[i].score;
      }
    } else {
      lastScore = poolBrackets[i].score;
    }
    char diff_str[8];
    if (poolTournamentBracket.tieBreak > 0) {
      snprintf(diff_str, sizeof(diff_str), "%4d", poolBrackets[i].tieBreakDiff);
    } else {
      memcpy(diff_str, "   ?", 5);
    }
    printf("│ %4d │ %-20.20s │ %5d │ %5d │ %6.2f%% │ %3s %c │ %5d │ %s │ %3d │ %3d │ %3d │ %3d │ %3d │ %3d │\n",
      rank,
      poolBrackets[i].name,
      poolBrackets[i].score,
      poolBrackets[i].maxScore,
      poolBrackets[i].wins * 100.0 / (pool_games_played() == 0 ? 1 : pool_games_played()),
      POOL_TEAM_SHORT_NAME(poolBrackets[i].winners[62]),
      poolTeams[poolBrackets[i].winners[62] - 1].eliminated ? 'N' : 'Y',
      poolBrackets[i].tieBreak,
      diff_str,
      poolBrackets[i].roundScores[0],
      poolBrackets[i].roundScores[1],
      poolBrackets[i].roundScores[2],
      poolBrackets[i].roundScores[3],
      poolBrackets[i].roundScores[4],
      poolBrackets[i].roundScores[5]
    );
  }
  printf("%s\n", bot);
  int gamesLeftCount = 63 - pool_games_played();
  if (gamesLeftCount > 0) {
    uint64_t possibleOutcomes = 2L << (gamesLeftCount - 1);
    printf("There are %d teams and %d games remaining, ",
      gamesLeftCount + 1, gamesLeftCount);
    pool_print_humanized(stdout, possibleOutcomes, 6);
    printf(" possible outcomes\n");
  }
}

POOLDEF void pool_scoredetail_report(const char *entryName) {
  if (poolBracketsCount == 0) {
    fprintf(stderr, ">>>> There are no entries in this pool. <<<<\n");
    return;
  }
  PoolBracket *bracket = NULL;
  for (size_t i = 0; i < poolBracketsCount; i++) {
    if (strstr(poolBrackets[i].name, entryName) != NULL) {
      bracket = &poolBrackets[i];
      break;
    }
  }
  if (bracket == NULL) {
    fprintf(stderr, "No entry found matching '%s'\n", entryName);
    return;
  }
  pool_bracket_score(bracket, &poolTournamentBracket);
  static const char *roundNames[] = {"R1", "R2", "R3", "R4", "F4", "Ch"};
  printf("Entry: %-20s  Score: %u\n", bracket->name, bracket->score);
  printf("┌──────┬────┬────────────┬────────────┬────────────┬─────────┬──────┐\n");
  printf("│ Game │ Rd │ Team 1     │ Team 2     │ Your Pick  │ Correct │  Pts │\n");
  printf("├──────┼────┼────────────┼────────────┼────────────┼─────────┼──────┤\n");
  for (uint8_t g = 0; g < POOL_NUM_GAMES; g++) {
    uint8_t round = pool_round_of_game(g);
    uint8_t team1 = 0, team2 = 0;
    pool_teams_of_game(g, round, &poolTournamentBracket, &team1, &team2);
    uint8_t pw = poolTournamentBracket.winners[g];
    uint8_t ew = bracket->winners[g];
    bool played = pw != 0;
    bool correct = played && ew == pw;
    uint32_t pts = 0;
    if (correct) {
      uint8_t loser = pool_loser_of_game(g, &poolTournamentBracket);
      pts = (*poolConfiguration.poolScorer)(bracket, pw, loser, round, g);
    }
    const char *t1name = team1 > 0 ? poolTeams[team1-1].shortName : "?";
    const char *t2name = team2 > 0 ? poolTeams[team2-1].shortName : "?";
    const char *ename  = ew   > 0 ? poolTeams[ew-1].shortName   : "?";
    if (!played) {
      printf("│ %4u │ %s │ %-10s │ %-10s │ %-10s │    -    │    - │\n",
             g, roundNames[round], t1name, t2name, ename);
    } else if (correct) {
      printf("│ %4u │ %s │ %-10s │ %-10s │ %-10s │ \033[32m%-9s\033[0m │ %4u │\n",
             g, roundNames[round], t1name, t2name, ename, "   ✓", pts);
    } else {
      printf("│ %4u │ %s │ %-10s │ %-10s │ %-10s │ \033[31m%-9s\033[0m │ %4u │\n",
             g, roundNames[round], t1name, t2name, ename, "   ✗", pts);
    }
  }
  printf("└──────┴────┴────────────┴────────────┴────────────┴─────────┴──────┘\n");
}

POOLDEF int pool_name_cmpfunc (const void * a, const void * b) {
  char alower[POOL_BRACKET_NAME_LIMIT];
  char blower[POOL_BRACKET_NAME_LIMIT];
  for(int i = 0; i < POOL_BRACKET_NAME_LIMIT; i++) alower[i] = tolower(((PoolBracket*)a)->name[i]);
  for(int i = 0; i < POOL_BRACKET_NAME_LIMIT; i++) blower[i] = tolower(((PoolBracket*)b)->name[i]);
  return strcmp(alower, blower);
}

POOLDEF void pool_entries_report(PoolReportFormat fmt) {
  for (size_t i = 0; i < poolBracketsCount; i++) {
    pool_bracket_score(&poolBrackets[i], &poolTournamentBracket);
  }
  if (pool_games_played() == 63) {
    qsort(poolBrackets, poolBracketsCount, sizeof(PoolBracket), pool_score_cmpfunc);
  } else {
    qsort(poolBrackets, poolBracketsCount, sizeof(PoolBracket), pool_name_cmpfunc);
  }

  if (fmt == PoolFormatJson) {
    printf("{");
    printf("\"pool\": {");
    printf("\"name\": \"%s\",", poolConfiguration.poolName);
    printf("\"gamesPlayed\": %d", pool_games_played());
    printf("},");
    printf("\"tournament\": {");
    printf("\"tieBreak\": %d,", poolTournamentBracket.tieBreak);
    printf("\"winners\": [");
    for (int g = 0; g < POOL_NUM_GAMES; g++) {
      if (g > 0) { printf(","); }
      printf("%d", poolTournamentBracket.winners[g]);
    }
    printf("]},");
    printf("\"entries\": [");
    for (size_t i = 0; i < poolBracketsCount; i++) {
      if (i > 0) { printf(","); }
      PoolBracket *e = &poolBrackets[i];
      printf("{");
      printf("\"name\": \"%s\",", e->name);
      printf("\"score\": %d,", e->score);
      printf("\"maxScore\": %d,", e->maxScore);
      printf("\"tieBreak\": %d,", e->tieBreak);
      printf("\"tieBreakDiff\": %d,", e->tieBreakDiff);
      printf("\"correctPicks\": %d,", e->wins);
      printf("\"roundScores\": [%d,%d,%d,%d,%d,%d],",
        e->roundScores[0], e->roundScores[1], e->roundScores[2],
        e->roundScores[3], e->roundScores[4], e->roundScores[5]);
      printf("\"winners\": [");
      for (int g = 0; g < POOL_NUM_GAMES; g++) {
        if (g > 0) { printf(","); }
        printf("%d", e->winners[g]);
      }
      printf("]}");
    }
    printf("]}");
    printf("\n");
    if (poolBracketsCount == 0) {
      fprintf(stderr, ">>>> There are no entries in this pool. <<<<\n");
    }
    return;
  }

  printf("%s: Entries Report\n", poolConfiguration.poolName);
  pool_print_entry(&poolTournamentBracket);
  for (size_t i = 0; i < poolBracketsCount; i++) {
    pool_print_entry(&poolBrackets[i]);
  }
  if (poolBracketsCount == 0) {
    fprintf(stderr, ">>>> There are no entries in this pool. <<<<\n");
  }
}

POOLDEF uint8_t pool_read_entry_to_bracket(const char *filePath, const char *entryName, size_t entryNameSize,
    PoolBracket *bracket, bool recordEliminations) {
  FILE *f = fopen(filePath, "rb");
  if (f == NULL) {
    fprintf(stderr, "[WARN] Could not open entry file %s: %s\n",
        filePath, strerror(errno));
    return 0;
  }
  if (entryNameSize > POOL_BRACKET_NAME_LIMIT) {
    fprintf(stderr, "Entry file name has to be less than %d chars long: %s\n",
        POOL_BRACKET_NAME_LIMIT, filePath);
    exit(1);
  }
  char buffer[1024];
  strncpy(bracket->name, entryName, entryNameSize);
  strncpy(bracket->name + entryNameSize, "", 1);
  char * line = fgets(buffer, 1023, f);
  uint8_t teamRounds[POOL_NUM_TEAMS] = {0};
  uint8_t resultsCount = 0;
  while (line != NULL) {
    // Turn the new line into a 0
    if ( buffer[strlen(buffer) - 1] == 10 ) { buffer[strlen(buffer)- 1] = 0; }

    // Skip lines starting with '#'
    if (buffer[0] == '#') {
      line = fgets(buffer, 1023, f);
      continue;
    }

    if (resultsCount == POOL_NUM_GAMES) {
      // reading tie break
      bracket->tieBreak = (uint8_t) atoi(buffer);
      break;
    } else if (resultsCount > POOL_NUM_GAMES) {
      fprintf(stderr, "Too many lines in entry file %s: %d\n", filePath, resultsCount);
      exit(1);
    } else {
      // Swap out play ins
      if (strncmp(buffer, "PI1", POOL_TEAM_SHORT_NAME_LIMIT) == 0 && strlen(poolConfiguration.pi1) > 0) {
        strcpy(buffer, poolConfiguration.pi1);
      }
      if (strncmp(buffer, "PI2", POOL_TEAM_SHORT_NAME_LIMIT) == 0 && strlen(poolConfiguration.pi2) > 0) {
        strcpy(buffer, poolConfiguration.pi2);
      }
      if (strncmp(buffer, "PI3", POOL_TEAM_SHORT_NAME_LIMIT) == 0 && strlen(poolConfiguration.pi3) > 0) {
        strcpy(buffer, poolConfiguration.pi3);
      }
      if (strncmp(buffer, "PI4", POOL_TEAM_SHORT_NAME_LIMIT) == 0 && strlen(poolConfiguration.pi4) > 0) {
        strcpy(buffer, poolConfiguration.pi4);
      }
      uint8_t teamNum = pool_team_num_for_short_name(buffer);
      if (teamNum == 0) {
        fprintf(stderr, "Could not determine team number of %s\n", buffer);
        exit(1);
      }
      uint8_t round = teamRounds[teamNum-1];
      size_t gameIndex = poolGameNumber[teamNum-1][round];
      if (bracket->winners[gameIndex] != 0) {
        PoolTeam prevWinner = poolTeams[bracket->winners[gameIndex]-1];
        fprintf(stderr, "Error in bracket %s: conflicting result %s for game %ld round %d, previous winner: %s\n",
            bracket->name, buffer, gameIndex, round + 1, prevWinner.name);
        exit(1);
      } else {
        bracket->winners[gameIndex] = teamNum;
        if (recordEliminations) {
          uint8_t loserNum = pool_loser_of_game(gameIndex, bracket);
          poolTeams[loserNum-1].eliminated = true;
        }
        teamRounds[teamNum-1] = teamRounds[teamNum-1] + 1;
      }
      bracket->wins += 1;
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
    fprintf(stderr, "[WARN] Can't open directory %s: %s\n",
      dirPath, strerror(errno));
    fprintf(stderr, "[WARN] There are no entries in this pool\n");
    return;
  }

  char entryFilePath[2048];

  while ((dp = readdir(dfd)) != NULL) {
    struct stat stbuf;
    sprintf(entryFilePath, "%s/%s", dirPath, dp->d_name);
    if ( stat(entryFilePath, &stbuf) == -1 ) {
      // .. do nothing?
      continue;
    }
    if ( S_ISDIR(stbuf.st_mode) ) {
      // skip directories
      continue;
    }
    char *ext = strrchr(entryFilePath, '.');
    if (!ext || strncmp(ext+1, "txt", 3) != 0) {
      // skip no extension or wrong extension
      continue;
    }
    PoolBracket *entry = &poolBrackets[poolBracketsCount++];
    uint8_t resultsAdded =
      pool_read_entry_to_bracket(entryFilePath, dp->d_name, strlen(dp->d_name) - 4, entry, false);
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
  // set defaults
  pool_defaults();

  FILE *f = fopen(filePath, "rb");
  if (f == NULL) {
    // Do nothing, use defaults
    fprintf(stderr, "[WARN] No config.txt in pool directory, using defaults.\n");
    return;
  }
  char buffer[1024];
  char * line = fgets(buffer, 1023, f);
  while (line != NULL) {
    // turn new lines into 0
    if ( buffer[strlen(buffer) - 1] == 10 ) { buffer[strlen(buffer)- 1] = 0; }
    if (strncmp(line, "name=", 5) == 0) {
      strncpy(poolConfiguration.poolName, line + 5, POOL_NAME_LIMIT);
    }
    if (strncmp(line, "roundScores=", strlen("roundScores=")) == 0) {
      if (sscanf(line, "roundScores=%u,%u,%u,%u,%u,%u",
        &poolConfiguration.roundScores[0],
        &poolConfiguration.roundScores[1],
        &poolConfiguration.roundScores[2],
        &poolConfiguration.roundScores[3],
        &poolConfiguration.roundScores[4],
        &poolConfiguration.roundScores[5]) != 6) {
          fprintf(stderr, "config.txt roundScores= line does not have 6 numerical values separated by commas\n");
          exit(1);
      }
    }
    if (strncmp(line, "scorerType=", 11) == 0) {
      if (strncmp(line+11, "Basic", 5) == 0) {
        // do nothing, default is basic
      } else if (strncmp(line+11, "UpsetMultiplier", 15) == 0) {
        poolConfiguration.scorerType = PoolScorerUpsetMultiplier;
      } else if (strncmp(line+11, "Upset", 5) == 0) {
        poolConfiguration.scorerType = PoolScorerUpset;
      } else if (strncmp(line+11, "JoshP", 5) == 0) {
        poolConfiguration.scorerType = PoolScorerJoshP;
      } else if (strncmp(line+11, "RelaxedSeedDiff", 15) == 0) {
        poolConfiguration.scorerType = PoolScorerRelaxedSeedDiff;
      } else if (strncmp(line+11, "SeedDiff", 8) == 0) {
        poolConfiguration.scorerType = PoolScorerSeedDiff;
      } else {
        fprintf(stderr, "config.txt scorerType %s is not known\n", line+11);
        exit(1);
      }
    }
    if (strncmp(line, "PI1=", 4) == 0) {
      strncpy(poolConfiguration.pi1, line + 4, POOL_TEAM_SHORT_NAME_LIMIT);
    }
    if (strncmp(line, "PI2=", 4) == 0) {
      strncpy(poolConfiguration.pi2, line + 4, POOL_TEAM_SHORT_NAME_LIMIT);
    }
    if (strncmp(line, "PI3=", 4) == 0) {
      strncpy(poolConfiguration.pi3, line + 4, POOL_TEAM_SHORT_NAME_LIMIT);
    }
    if (strncmp(line, "PI4=", 4) == 0) {
      strncpy(poolConfiguration.pi4, line + 4, POOL_TEAM_SHORT_NAME_LIMIT);
    }
    if (strncmp(line, "fee=", 4) == 0) {
      int fee = atoi(line + 4);
      if (fee <= 0) {
        fprintf(stderr, "config.txt fee '%s' must be a positive integer\n", line+4);
        exit(1);
      }
      //fprintf(stderr, "config.txt fee = %d '%s'\n", fee, line+4);
      poolConfiguration.fee = (uint8_t) fee;
    }
    if (strncmp(line, "payouts=", 8) == 0) {
      uint32_t offset = 0;
      char *payoutValues = line + 8;
      //printf("PARSING PAYOUTS FROM %s\n", payoutValues);
      char *token = strtok(payoutValues, ",");
      if (token == NULL) {
        fprintf(stderr, "config.txt payouts could not be parsed, does it have commas?\n");
        exit(1);
      }
      int sum = 0;
      while (token != NULL) {
        //printf("PARSING PAYOUTS, GOT TOKEN: [%s]\n", token);
        if (offset >= POOL_MAX_PAYOUTS) {
          fprintf(stderr, "config.txt payouts has too many payouts configured, max allowed is %d\n", POOL_MAX_PAYOUTS);
          exit(1);
        }
        int amount = atoi(token);
        if (amount > 0) {
          sum += amount;
          poolConfiguration.payouts[offset] = amount;
        } else if (amount == -1) {
          poolConfiguration.payouts[offset] = amount;
        } else {
          fprintf(stderr, "config.txt payouts has an illegal value %d (parsed from '%s'). Values must be a positive integer or -1\n", amount, token);
          exit(1);
        }
        offset++;
        token = strtok(NULL, ",");
      }
      if (sum != 100) {
        fprintf(stderr, "config.txt payouts don't add up to exactly 100\n");
        exit(1);
      }
      //printf("PAYOUT #1: %d\n", poolConfiguration.payouts[0]);
      //printf("PAYOUT #2: %d\n", poolConfiguration.payouts[1]);
      //printf("PAYOUT #3: %d\n", poolConfiguration.payouts[2]);
      //printf("PAYOUT #4: %d\n", poolConfiguration.payouts[3]);
    }
    line = fgets(buffer, 1023, f);
  }
  poolConfiguration.poolScorer = pool_get_scorer_function(poolConfiguration.scorerType);
}

POOLDEF uint8_t pool_games_played(void) {
  return poolTournamentBracket.wins;
}

POOLDEF void pool_read_team_file(const char *filePath) {
  FILE *f = fopen(filePath, "rb");
  if (f == NULL) {
    fprintf(stderr, "Could not open team file %s: %s\n",
        filePath, strerror(errno));
    exit(1);
  }
  char buffer[1024];
  int regionCount = 0;
  for (uint8_t i = 0; i < POOL_NUM_TEAMS; i++) {
    char *line = fgets(buffer, 1023, f);
    if (line == NULL) {
      fprintf(stderr, "Could not read a line from file %s: %s\n",
          filePath, strerror(errno));
      exit(1);
    }
    while (buffer[0] == '#' || strncmp(buffer, "Region:", 7) == 0) {
      if (strncmp(buffer, "Region:", 7) == 0) {
        if (regionCount < 4) {
          char *nameStart = buffer + 7;
          while (*nameStart == ' ') nameStart++;
          size_t nameLen = strlen(nameStart);
          while (nameLen > 0 && (nameStart[nameLen-1] == '\n' || nameStart[nameLen-1] == '\r')) {
            nameStart[--nameLen] = 0;
          }
          strncpy(poolConfiguration.regionNames[regionCount], nameStart, 31);
          poolConfiguration.regionNames[regionCount][31] = 0;
        }
        regionCount++;
      }
      line = fgets(buffer, 1023, f);
      if (line == NULL) {
        fprintf(stderr, "Could not read a line from file %s: %s\n",
          filePath, strerror(errno));
        exit(1);
      }
    }

    // Turn new line into end of string marker
    if ( buffer[strlen(buffer) - 1] == 10 ) { buffer[strlen(buffer)- 1] = 0; }

    PoolTeam *team = &poolTeams[i];
    team->seed = poolSeeds[i];
    int vars_filled = sscanf(buffer, "%32[^,],%3s", team->name, team->shortName);
    if (vars_filled != 2) {
      fprintf(stderr, "Could not read name and short name from line %s\n", buffer);
      exit(1);
    }
    // Swap in PI[1234] accordingly
    if (strcmp(team->shortName, "PI1") == 0 && strlen(poolConfiguration.pi1) > 0) {
      strncpy(team->shortName, poolConfiguration.pi1, POOL_TEAM_SHORT_NAME_LIMIT);
    }
    if (strcmp(team->shortName, "PI2") == 0 && strlen(poolConfiguration.pi2) > 0) {
      strncpy(team->shortName, poolConfiguration.pi2, POOL_TEAM_SHORT_NAME_LIMIT);
    }
    if (strcmp(team->shortName, "PI3") == 0 && strlen(poolConfiguration.pi3) > 0) {
      strncpy(team->shortName, poolConfiguration.pi3, POOL_TEAM_SHORT_NAME_LIMIT);
    }
    if (strcmp(team->shortName, "PI4") == 0 && strlen(poolConfiguration.pi4) > 0) {
      strncpy(team->shortName, poolConfiguration.pi4, POOL_TEAM_SHORT_NAME_LIMIT);
    }
    // Check if we already read this
    for (uint8_t t = 0; t < i; t++) {
      if (strcmp(poolTeams[i].name, poolTeams[t].name) == 0) {
        fprintf(stderr, "Duplicate team name %s for team %d and %d\n", poolTeams[i].name, i+1, t+1);
        exit(1);
      }
      if (strcmp(poolTeams[i].shortName, poolTeams[t].shortName) == 0) {
        fprintf(stderr, "Duplicate team shortName |%s| for team %d and %d\n", poolTeams[i].shortName, i+1, t+1);
        exit(1);
      }
    }
  }
  if (regionCount != 0 && regionCount != 4) {
    fprintf(stderr, "teams.txt must contain exactly 0 or 4 Region: lines, found %d\n", regionCount);
    exit(1);
  }
  fclose(f);
}
#endif // POOL_IMPLEMENTATION
