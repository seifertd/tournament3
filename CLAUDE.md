# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Building

```bash
make                # builds pool, testpool, benchmark, scoredetail
make clean all      # clean then rebuild
CC=clang make clean all  # use clang instead of cc
```

Alternatively, using the shell script:

```bash
./build.sh
CC=clang ./build.sh
```

Flags used: `-Wall -Wextra -pedantic -std=c99 -O3 -lm` (clang uses `-Wno-string-concatenation`; gcc uses `-Wno-stringop-truncation`).

## Running

```bash
./pool -h                          # help
./pool -d <dir> teams              # show configured teams
./pool -d <dir> entries            # show entry brackets
./pool -d <dir> scores             # show current scores
./pool -d <dir> poss               # run possibilities report (slow)
./pool -d <dir> ffour              # final four report
./pool -d <dir> -p poss            # poss with progress display
./pool -d <dir> -b N -n M -f bin poss   # parallel batch N of M, write binary
./pool -d <dir> -r poss            # combine saved bin files into final report
./parallel.sh <dir>                # run 16 parallel poss processes
```

## Testing

```bash
./testpool         # integration test using test/fifty_entries
./benchmark        # scoring performance benchmark using test/benchmark
./scoredetail      # per-game score detail for a single entry (hardcoded paths in source)
```

There are no automated test assertions — `testpool` exercises all major report paths and prints output for manual inspection.

## Architecture

The library lives entirely in **`pool.h`** using the single-file header pattern:
- Without `#define POOL_IMPLEMENTATION`: only declarations/types are included (for consumers that just need the API).
- With `#define POOL_IMPLEMENTATION`: full implementation is compiled in.

All public functions are declared `static inline` via the `POOLDEF` macro.

**Files:**
- `pool.h` — the entire library: data structures, globals, all scorer implementations, DFS possibilities engine, report generators, file I/O
- `pool.c` — CLI frontend; parses args with `getopt`, calls `pool_initialize()` and the appropriate report function
- `testpool.c` — integration harness; calls all major API functions against `test/fifty_entries`
- `benchmark.c` — times `pool_bracket_score()` iterations against `test/benchmark`
- `scoredetail.c` — ad hoc debug tool printing per-game scoring against `test/cbs_experts_2023`

**Key globals in pool.h:**
- `poolTeams[64]` — team data (name, short name, seed, eliminated)
- `poolBrackets[1024]` — all loaded entry brackets
- `poolTournamentBracket` — the actual game results bracket
- `poolConfiguration` — scorer type, round multipliers, payout config, PI mappings

**Pool directory layout (input files):**
```
<dir>/config.txt      # pool name, scorer, roundScores, fee, payouts, PI mappings
<dir>/teams.txt       # 64 teams: "Long Name,SHT" in seed order per region
<dir>/results.txt     # actual game winners, one short name per line
<dir>/entries/        # one .txt file per entrant, same format as results.txt
```

**Possibilities report (DFS engine):**
The core algorithm is a recursive DFS over all remaining games. For each game it tries both possible winners, adds the score delta to each entry's `possibleScore`, recurses, then subtracts it back. At leaf nodes it ranks all entries and updates min/max rank, timesWon, timesTied, champCounts. The histogram optimization (`POOL_FREQ_SIZE = 4096`) avoids re-sorting by tracking score frequencies.

Parallelization splits the DFS by fixing the outcomes of the first `log2(N)` games across N processes (`-b 0..N-1 -n N`), writing partial binary stats files (`poss_N_of_M.bin`), then combining with `-r`.

**Scorers** (`PoolScorerType` enum): Basic, Upset, JoshP, SeedDiff, RelaxedSeedDiff, UpsetMultiplier. Each takes `(bracket, winner, loser, round, game)` and returns a `uint32_t` point value. Selected via inline switch in the DFS hot path (no function pointer indirection).

**Game indexing:** 63 games total (0–62). Round 1 = games 0–31, Round 2 = 32–47, Round 3 = 48–55, Round 4 = 56–59, Round 5 = 60–61, Championship = 62. The `poolGameNumber[64][6]` table maps each team × round to the game index where that team plays.
