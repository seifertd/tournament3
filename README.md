Tournament 3
-------------
Back to basics NCAA basketball pool software implemented in pure C for speed.
No nonsensical bit twiddling.  40.2M bracket scores per second. 

Implemented as a single file header for command line pool management.
This may change at some point, not sure what benefit this has as
the use case I was thinking of no longer applies.

Scorers
-------
The pool configuration specifies the scorer to use in the pool.
Each scorer uses as input the bracket being scored, the round number
and the game number. In addition to the scorer, the pool configuration
specifies 6 round multipliers that the scorer can use. The default
multipliers are 1, 2, 4, 8, 16, 32
There are 3 supported scorers:

1. Basic: each correct pick is worth a constant amount - the
   round multiplier configured for that round.
1. Upset: each correct pick is worth the round multiplier
   for the game's round plus the seed number of the winning team.
1. SeedDiff: each correct pick is worth the round multiplier
   for the game's round plus the difference in seeds of the 
   winning team and the losing team. The bonus points only 
   apply if the loser was picked correctly and the winner's
   seed is greater than the loser's seed.

The choice of scorer can affect the performance of the pool
possibilities report.

Possibilities Report
---------------------
We want to generate all possible outcomes and score all the entries
against them. Set up a DFS with inputs:
* games - array of game numbers remaining
* gamesLeft - size of this array
* game - game under review
* stats - array of stats structs, one per entry
  * maxRank, minRank, maxScore, timesWon, timesTied, possibleScore, champCounts[], bracket
  * possibleScore is set to current bracket score

Algo:
* determine the two teams in games[game]
* Assume winner is the first team, calculate points awarded to each bracket, add
  to possibleScore
* Recurse with game += 1
* Repeat the above with winner assumed to be second team

max recursion depth would be = number of games remaining

Parallelization of Possibilities Report
---------------------------------------
We can parallelize the above to a power of 2 processes/servers/cores/etc if the
current tournament bracket is advanced and the algo run from there, then results
collected and combined.

2 cores:
  * 1st core assumes game 1 winner is team1
  * 2nd core assumes game 1 winner is team2

4 cores:
  * 1st core assumes game 1 winner is team1, game 2 winner is team1
  * 2nd core assumes game 1 winner is team2, game 2 winner is team1
  * 3nd core assumes game 1 winner is team1, game 2 winner is team2
  * 4th core assumes game 1 winner is team2, game 2 winner is team2

8 cores:
  * 1st core assumes game 1 winner is team1, game 2 winner is team1, game 3 winner is team1
  * 2nd core assumes game 1 winner is team2, game 2 winner is team1, game 3 winner is team1
  * 3nd core assumes game 1 winner is team1, game 2 winner is team2, game 3 winner is team1
  * 4th core assumes game 1 winner is team2, game 2 winner is team2, game 3 winner is team1
  * 5th core assumes game 1 winner is team1, game 2 winner is team1, game 3 winner is team2
  * 6th core assumes game 1 winner is team2, game 2 winner is team1, game 3 winner is team2
  * 7th core assumes game 1 winner is team1, game 2 winner is team2, game 3 winner is team2
  * 8th core assumes game 1 winner is team2, game 2 winner is team2, game 3 winner is team2

etc

Game Index
-----------
Game numbers by round and what game numbers teams will play in
as they advance. Read across and up.

    Game indexes
                    Round
    Teams     1  2   3   4   5   6
    t1 t2:    0 32  48  56  60  62
    t3 t4:    1  +   |   |   |
    t5 t6:    2 33 --+   |   |
    t7 t8:    3  +       |   |
    t9 t10:   4 34  49 --+   |
    t11 t12:  5  +   |       |
    t13 t14:  6 35 --+       |
    t15 t16:  7  +           |
    t17 t18:  8 36  50  57 --+
    t19 t20:  9
    t21 t22: 10 37
    t23 t24: 11
    t25 t26: 12 38  51
    t27 t28: 13 
    t29 t30: 14 39
    t31 t32: 15 

    t33 t34: 16 40  52  58  61
    t35 t36: 17
    t37 t38: 18 41
    t39 t40: 19
    t41 t42: 20 42  53
    t43 t44: 21
    t45 t46: 22 43
    t47 t48: 23
    t49 t50: 24 44  54  59
    t51 t52: 25
    t53 t54: 26 45
    t55 t56: 27
    t57 t58: 28 46  55
    t59 t60: 29
    t61 t62: 30 47
    t63 t64: 31
