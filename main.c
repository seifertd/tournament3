#include <stdio.h>
#include <time.h>

#define POOL_IMPLEMENTATION
#include "pool.c"

int main(int argc, const char* argv) {
  /*
  const char *filePath = "./2023/teams.txt";
  pool_read_team_file(filePath);
  pool_add_entries_in_dir("./2023/entries");
  pool_entries_report();
  uint8_t gamesPlayed = pool_read_entry_to_bracket("./2023/results.txt", "Tourney", 7, &poolTournamentBracket);
  printf("Tournament Results:\n");
  pool_print_entry(&poolTournamentBracket);
  */
  //pool_initialize("2023");
  pool_initialize("test/benchmark");
  printf("Initialized pool, there are %d entries\n", pool_brackets_count);
  printf("Pool Name: %s\n", poolConfiguration.poolName);
  printf("Pool Scorer: %d\n", poolConfiguration.scorerType);
  printf("Pool Round Multipliers: %d %d %d %d %d %d\n", 
      poolConfiguration.roundMultipliers[0],
      poolConfiguration.roundMultipliers[1],
      poolConfiguration.roundMultipliers[2],
      poolConfiguration.roundMultipliers[3],
      poolConfiguration.roundMultipliers[4],
      poolConfiguration.roundMultipliers[5]);
  printf("Tournament Results:\n");
  pool_print_entry(&poolTournamentBracket);
  pool_entries_report();
  pool_score_report();

  printf("Running Scoring Benchmark:\n");
  clock_t start_t, end_t;
  double total_t;
  int iters = 1000000;
  
  start_t = clock();
  for (int i = 0; i < iters; i++) {
    for (int b = 0; b < pool_brackets_count; b++) {
      PoolBracket *bracket = &poolBrackets[b];
      pool_bracket_score(bracket,
        poolConfiguration.scorerType,
        poolConfiguration.roundMultipliers);
    }
  }
  end_t = clock();
  total_t = (double)(end_t - start_t)/CLOCKS_PER_SEC;
  double scores_per_sec = iters * pool_brackets_count / total_t;
  printf("%d scores of %d brackets in %f seconds, %f scores/sec\n",
      iters, pool_brackets_count, total_t, scores_per_sec);
}
