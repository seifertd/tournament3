#include <stdio.h>
#include <string.h>

#define POOL_IMPLEMENTATION
#include "pool.h"

int main(void) {
  PoolBracket entry = {0};
  pool_read_config_file("./test/cbs_experts_2023/config.txt");
  pool_read_team_file("./test/cbs_experts_2023/teams.txt");
  pool_read_entry_to_bracket("./test/cbs_experts_2023/entries/Matt Norlander.txt", "Matt", 4, &entry, false);
  pool_read_entry_to_bracket("./test/cbs_experts_2023/results.txt", "Tourney", 7, &poolTournamentBracket, true);
  //pool_read_entry_to_bracket("./test/fifty_entries/results.txt", "Tourney", 7, &poolTournamentBracket, true);
 
  printf("Results:\n");
  pool_print_entry(&poolTournamentBracket);
  printf("entry32:\n");
  pool_print_entry(&entry);
  pool_bracket_score(&entry, &poolTournamentBracket);
  printf("Score: %d\n", entry.score);
  printf("Max Score: %d\n", entry.maxScore);
  uint32_t total = 0;
  uint32_t maxScore = 0;
  for (uint8_t g = 0; g < POOL_NUM_GAMES; g++) {
    uint8_t team1 = 0;
    uint8_t team2 = 0;
    uint8_t round = pool_round_of_game(g);
    pool_teams_of_game(g, round, &entry, &team1, &team2); 
    uint32_t score = 0;
    if (poolTournamentBracket.winners[g] != 0) {
      if (entry.winners[g] == poolTournamentBracket.winners[g]) {
        score = (*poolConfiguration.poolScorer)(&entry, &poolTournamentBracket, round, g);
        maxScore += score;
      }
    } else {
      if (!poolTeams[entry.winners[g] - 1].eliminated) {
        maxScore += (*poolConfiguration.poolScorer)(&entry, &poolTournamentBracket, round, g);
      }
    }
    /*
    if (entry.winners[g] == poolTournamentBracket.winners[g]) {
      score = (*poolConfiguration.poolScorer)(&entry, &poolTournamentBracket, round, g);
    }
    if (!poolTeams[entry.winners[g]-1].eliminated &&
      (entry.winners[g] == poolTournamentBracket.winners[g] || poolTournamentBracket.winners[g] == 0)) {
      maxScore += (*poolConfiguration.poolScorer)(&entry, &poolTournamentBracket, round, g);
    }
    */
    total += score;
    PoolTeam *t1 = team1 > 0 ? &poolTeams[team1-1] : NULL;
    PoolTeam *t2 = team2 > 0 ? &poolTeams[team2-1] : NULL;
    printf("Game %u:", g);
    printf(STRIKE(team1, poolTournamentBracket.winners[g], "%s"), POOL_TEAM_SHORT_NAME(team1));
    printf(" (%d) vs ", t1 ? t1->seed : 0);
    printf(STRIKE(team2, poolTournamentBracket.winners[g], "%s"), POOL_TEAM_SHORT_NAME(team2));
    printf(" (%d) => ", t2 ? t2->seed : 0);
    printf(STRIKE(entry.winners[g], poolTournamentBracket.winners[g], "%s"), POOL_TEAM_SHORT_NAME(entry.winners[g]));
    printf(" =? %s: score: %u/%u/%u\n",
           POOL_TEAM_SHORT_NAME(poolTournamentBracket.winners[g]),
           score, total, maxScore);
  }
  return 0;
}
