#include <stdio.h>
#include <time.h>

#define POOL_IMPLEMENTATION
#include "pool.h"

int main(int argc, const char* argv) {
  pool_initialize("2023");
  printf("Running Scoring Benchmark:\n");
  clock_t start_t, end_t;
  double total_t;
  int iters = 1000000;
  
  start_t = clock();
  for (int i = 0; i < iters; i++) {
    for (int b = 0; b < poolBracketsCount; b++) {
      PoolBracket *bracket = &poolBrackets[b];
      pool_bracket_score(bracket,
        &poolTournamentBracket);
    }
  }
  end_t = clock();
  total_t = (double)(end_t - start_t)/CLOCKS_PER_SEC;
  double scores_per_sec = iters * poolBracketsCount / total_t;
  printf("%d scores of %d brackets in %f seconds, %f scores/sec\n",
      iters, poolBracketsCount, total_t, scores_per_sec);
  return 0;
}
