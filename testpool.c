#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define POOL_IMPLEMENTATION
#include "pool.h"

int main(void) {
  pool_initialize("test/fifty_entries");
  printf("Initialized pool, there are %d entries\n", poolBracketsCount);
  printf("Pool Name: %s\n", poolConfiguration.poolName);
  printf("Pool Scorer: %d\n", poolConfiguration.scorerType);
  printf("Pool Round Multipliers: %d %d %d %d %d %d\n", 
      poolConfiguration.roundScores[0],
      poolConfiguration.roundScores[1],
      poolConfiguration.roundScores[2],
      poolConfiguration.roundScores[3],
      poolConfiguration.roundScores[4],
      poolConfiguration.roundScores[5]);
  pool_team_report();
  pool_entries_report();
  pool_score_report();
  printf("Tournament Results:\n");
  pool_print_entry(&poolTournamentBracket);
  pool_possibilities_report(PoolFormatText, true, 0, 1, false);

  // Advance pool to final four
  srand(time(NULL));
  rand();
  uint8_t gameNum = pool_games_played();
  size_t gamesRemaining = 63 - gameNum;
  while (gamesRemaining > 3) {
    uint8_t t1, t2;
    uint8_t round = pool_round_of_game(gameNum);
    pool_teams_of_game(gameNum, round, &poolTournamentBracket, &t1, &t2);
    if (rand() % 2 == 0) {
      poolTournamentBracket.winners[gameNum] = t1;
      poolTeams[t2-1].eliminated = true;
    } else {
      poolTournamentBracket.winners[gameNum] = t2;
      poolTeams[t1-1].eliminated = true;
    }
    poolTournamentBracket.wins += 1;
    gamesRemaining -= 1;
    gameNum += 1;
  }
  printf("Random Final Four:\n");
  pool_print_entry(&poolTournamentBracket);
  pool_final_four_report();

  return 0;
}
