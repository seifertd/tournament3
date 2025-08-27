#define POOL_IMPLEMENTATION
#include "pool.h"
#include <stdio.h>
#include <assert.h>

void test_min_seed_finder() {
    printf("=== Testing pool_find_min_possible_seed ===\n");
    
    // Test first round games (should return actual seeds)
    printf("First round tests:\n");
    for (int game = 0; game < 4; game++) {
        uint8_t seed1 = pool_find_min_possible_seed(game, 0, true);
        uint8_t seed2 = pool_find_min_possible_seed(game, 0, false);
        printf("Game %d: team1 min seed = %d, team2 min seed = %d\n", game, seed1, seed2);
    }
    
    // Test second round games (Round of 32)
    printf("\nSecond round tests:\n");
    for (int game = 32; game < 36; game++) {
        uint8_t seed1 = pool_find_min_possible_seed(game, 1, true);
        uint8_t seed2 = pool_find_min_possible_seed(game, 1, false);
        printf("Game %d: team1 min seed = %d, team2 min seed = %d\n", game, seed1, seed2);
    }
    
    // Test Sweet 16 games
    printf("\nSweet 16 tests:\n");
    for (int game = 48; game < 52; game++) {
        uint8_t seed1 = pool_find_min_possible_seed(game, 2, true);
        uint8_t seed2 = pool_find_min_possible_seed(game, 2, false);
        printf("Game %d: team1 min seed = %d, team2 min seed = %d\n", game, seed1, seed2);
    }
}

void test_relaxed_scorer_complete_bracket() {
    printf("\n=== Testing relaxed scorer with complete results ===\n");
    
    PoolBracket bracket = {0};
    PoolBracket results = {0};
    
    // Set up a first round game: 1 seed vs 16 seed
    bracket.winners[0] = 0;  // 1 seed wins
    results.winners[0] = 0;  // Actual result: 1 seed wins
    
    uint32_t score = pool_relaxed_seed_diff_scorer(&bracket, &results, 0, 0);
    printf("1 seed beats 16 seed (correct prediction): score = %d\n", score);
    printf("Expected: %d (base) + 0 (no upset bonus) = %d\n", 
           poolConfiguration.roundScores[0], poolConfiguration.roundScores[0]);
    
    // Test upset scenario
    bracket.winners[1] = 1;  // 16 seed wins (team 34 has seed 16)
    results.winners[1] = 1;  // Actual result: 16 seed wins
    
    score = pool_relaxed_seed_diff_scorer(&bracket, &results, 0, 1);
    printf("16 seed beats 1 seed (correct upset prediction): score = %d\n", score);
    printf("Expected: %d (base) + %d (16-1 bonus) = %d\n", 
           poolConfiguration.roundScores[0], 16-1, poolConfiguration.roundScores[0] + 15);
}


void test_bracket_with_partial_results() {
    printf("\n=== Testing with partially complete bracket ===\n");
    
    PoolBracket bracket = {0};
    PoolBracket partialResults = {0};
    
    // Fill in some first round results
    partialResults.winners[0] = 1;  // 1 seed advances
    partialResults.winners[1] = 3;  // 8 seed advances
    partialResults.winners[2] = 5;  // 5 seed advances
    partialResults.winners[3] = 7;  // 4 seed advances
    
    // Set up bracket predictions for second round
    bracket.winners[0] = 2;
    bracket.winners[1] = 4;
    bracket.winners[2] = 6;
    bracket.winners[3] = 8;
    bracket.winners[32] = 2;  // Predict 16 seed beats 1 seed in round 2
    
    // Test second round game that hasn't been played
    uint32_t score = pool_relaxed_seed_diff_scorer(&bracket, &partialResults, 1, 32);
    printf("Round 2: Predicting 16 seed beats 1 seed (game not played): score = %d\n", score);
    
    // In this case, we know the actual teams (1 seed vs 16 seed)
    // Max bonus would be 16 - 1 = 15
    printf("Expected: %d (base) + %d (max bonus 16-1) = %d\n", 
           poolConfiguration.roundScores[1], 15, poolConfiguration.roundScores[1] + 15);
}

int main() {
    printf("Testing Relaxed Seed Diff Scorer\n");
    printf("================================\n");
    
    // Initialize with defaults
    pool_defaults();
    poolConfiguration.scorerType = PoolScorerRelaxedSeedDiff;
    poolConfiguration.poolScorer = pool_get_scorer_function(poolConfiguration.scorerType);
    
    // Set up teams with seeds
    for (int i = 0; i < POOL_NUM_TEAMS; i++) {
        poolTeams[i].seed = poolSeeds[i];
        sprintf(poolTeams[i].name, "Team %d (Seed %d)", i+1, poolSeeds[i]);
        sprintf(poolTeams[i].shortName, "T%d", i+1);
        poolTeams[i].eliminated = false;
    }
    
    // Print first few teams for reference
    printf("First 8 teams:\n");
    for (int i = 0; i < 8; i++) {
        printf("Team %d: %s (seed %d)\n", i+1, poolTeams[i].name, poolTeams[i].seed);
    }
    printf("\n");
    
    // Run tests
    test_min_seed_finder();
    test_relaxed_scorer_complete_bracket();
    test_bracket_with_partial_results();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
