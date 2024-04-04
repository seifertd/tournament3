Tournament 3
-------------
Back to basics NCAA march madness basketball pool software implemented in pure C for speed.
No nonsensical bit twiddling.  40.2M bracket scores per second. 

Implemented as a single file header for command line pool management.
This may change at some point, not sure what benefit this has as
the use case I was thinking of no longer applies.

Building
--------

Run

```console
$ ./build.sh
$ ./pool -h
```

Build script uses value of CC environment variable or `cc` if it is not set
as the compiler.

Use clang:

```console
$ CC=clang ./build.sh
```

Tested on Linux Mint, Ubuntu, Windows 11 (MinGW), Windows 11 (WSL) and Mac OS X (M1).

Running a Pool
--------------
You can use this software to run your office or friend group NCAA pool.

1. Create a directory on your computer to hold the pool files.
1. Create the `config.txt` file and set your pool's name, scoring method
   and round multipliers.
   1. If you are running this pool for profit, also fill in the `fee`
      and `payouts` lines.
   1. The `fee` is the number of currency units you collect per entry.
   1. The `payouts` line is a comma separated list of no more than
      4 integers which specify the payouts by rank.
      1. positive numbers specify a percentage of the total fees collected
      1. -1 specifies that rank gets the entry fee back.
      1. The sum of the percentages has to equal exactly 100
1. Create the `teams.txt` file in the directory. See the `./pool -h` for a
   link to a sample you can edit.
   1. For the four play in games, use `PI1`, `PI2`, `PI3` and `PI4` as the
      short name. The long name is irrelevant
1. Collect entries
   1. You can hand out forms, set up a pool on one of the free websites
      or send out the included bracket HTML entry collector page via email.
   1. The HTML entry collector needs to be edited with the current year's
      teams.
      1. New for 2024: there is a ruby script to generate the web entry collector
         from the teams file. You need a working ruby install and can run it as
         follows: `ruby web/make_bracket.rb ./2024/teams.txt ./web/2024_bracket.html ./2024/logo.svg`
         * 1st argument: the location of the generator script in this repo
         * 2nd argument: the location of the teams file
         * 3rd argument: location where to save the bracket
         * 4th argument: the location of a logo svg file in urldata format
   1. Here is what the HTML entry collector looks like: [Latest NCAA Tournament Bracket](https://seifertd.github.io/tournament3/web/2024_bracket.html)
1. Create the `entries` subfolder and copy/create/save any entry files you
   got from the pool entrants. 
1. Generate an entries report and send it out to everyone for confirmation:
   `./pool -d mypool entries`
1. Handle the play in games.
   1. As the play in games are played, add to
      the pool `config.txt` file lines that map the place holder short
      team names (`PI[1234]`) with a new short name matching the winner.
      You will end up with lines like:
      ```
      PI1=Wag
      PI2=Cdo
      PI3=GrS
      PI4=CSt
      ```
   1. Make sure to change the long name of the play in teams in the
      `teams.txt` file, but don't modify the `PI[1234]` short names.
1. As games are played in the tournament, record the winners in the 
   `results.txt` file, one team short name per line. For play in games,
   you can use either the original `PI[1234]` short names or the mapped
   short names of the play in from the step above.
1. Run the scores report: `./pool -d mypool scores` until the first round
   is complete.
1. Run your first possibilities report as soon as your machine
   can handle it: `./pool -d mypool poss` and pass
   around the results via email, slack, discord or whatever.

Scorers
-------
The pool configuration specifies the scorer to use in the pool.
Each scorer uses as input the bracket being scored, the round number
and the game number. In addition to the scorer, the pool configuration
specifies 6 round multipliers that the scorer can use. The default
multipliers are 1, 2, 4, 8, 16, 32
These are the supported scorers:

1. Basic: each correct pick is worth a constant amount - the
   round multiplier configured for that round.
1. Upset: each correct pick is worth the round multiplier
   for the game's round plus the seed number of the winning team.
1. SeedDiff: each correct pick is worth the round multiplier
   for the game's round plus the difference in seeds of the 
   winning team and the losing team. The bonus points only 
   apply if the loser was picked correctly and the winner's
   seed is greater than the loser's seed.
1. RelaxedSeedDiff: each correct pick is worth the round multiplier
   for the game's round plus the difference in seeds of the 
   winning team and the losing team. The bonus points only 
   apply if winner's seed is greater than the loser's seed.
   This differs from SeedDiff in that the loser in the game
   need not have been picked correctly.
1. JoshP: each correct pick is worth the round multiplier
   for the game's round multiplied by the seed number of the
   winning team.

The choice of scorer can affect the performance of the pool
possibilities report.

Possibilities Report
---------------------

M1 mac can analyze 2.147B possible outcomes against 50 entries
in 37 minutes once there are 32 teams remaining.

```
Supercalifragilistic Pool: Possibilities Report
There are 32 teams and 31 games remaining,  2.147B possible outcomes
DFS BPS:       968207 100% ELAPSED: 00:36:58
            Min  Max  Curr  Max    Win   Times  Times
    Name   Rank Rank Score Score Chance   Won    Tied Top Champs
   entry23    1   29   193   497  17.93   385M 11.17M Cat,foe,wil,ane,hea
   entry26    1   36   169   521  15.94 342.3M 8.821M mor,cir,mas,ten,gri
   entry30    1   29   184   474  14.92 320.3M 11.19M slu,dis,dee,mus,hea
   entry32    1   31   184   420   7.61 163.5M 6.399M lan,pos,dur,wid,sho
   entry33    1   38   161   460   6.53 140.3M 5.080M fan,cla,ove,can,ane
    entry7    1   37   165   444   5.13   110M 5.036M lan,aba,ext,spl,Cha
    entry3    1   42   150   460   4.70 100.9M 4.543M dum,can,ten,ane,pos
    entry9    1   33   169   439   4.43 95.10M 4.895M Cha,mus,ane,Wis,can
    entry4    1   31   180   396   4.26 91.41M 4.098M moo,spl,ext,wid,aba
   entry14    1   41   165   406   4.24 91.14M 4.179M dee,dum,mas,gri,Wis
   entry47    1   44   155   380   2.57 55.11M 2.515M wil,mas,wid,pos,gri
   entry50    1   36   167   423   2.11 45.27M 2.912M Cha,sho,aba,rap,pos
   entry28    1   44   147   438   1.66 35.60M 1.718M mus,cir,lan,rap,gri
   entry21    1   42   154   403   1.55 33.21M 1.777M mus,hea,spl,Wis,rap
   entry19    1   41   158   363   1.20 25.79M 1.701M cla,spl,wid,sho,gri
   entry38    1   40   161   368   1.19 25.66M 1.612M rap,wil,sho,spl,wid
    entry6    1   48   121   423   0.82 17.61M 1.025M ten,ext,dur,foe,cir
   entry13    1   48   128   352   0.40 8.512M 512.9K cir,Wis,pin,ove,dum
   entry20    1   44   146   333   0.31 6.577M 587.7K dee,pin,ten,cla,ove
   entry10    1   43   150   345   0.24 5.236M   477K sho,cla,ext,cir,aba
   entry36    1   49   119   392   0.12 2.644M 213.4K aba,dur,dis,wid,spl
   entry16    1   44   146   353   0.10 2.212M 208.7K moo,foe,mas,Wis,ext
   entry41    1   40   155   302   0.06 1.254M 186.9K gri,aba,wid,pos,spl
   entry29    1   50   102   303   0.05 1.042M 86.31K mas,spl,ext,aba,pos
   entry37    1   48   133   335   0.03 630.7K 107.5K ten,spl,aba,gri,foe
   entry46    1   50   122   320   0.02 497.2K 62.95K mus,dur,ten,mas,gri
    entry8    1   50   101   311   0.01 232.3K 23.14K rap,pos,pin,can,gri
   entry31    1   50   100   318   0.01 166.9K 20.40K moo,ane,Wis,ext,wil
   entry34    1   50   124   272   0.00 46.97K 11.22K wid,mas,pin,ove,gri
   entry12    1   50   103   267   0.00   2720    752 rap,ane,ext
   entry27    1   50   122   255   0.00     29     64 cir,gri,rap,dur,ove
   entry17    2   50   114   291   0.00      0      0
    entry1    2   48   136   289   0.00      0      0
   entry43    2   49   128   284   0.00      0      0
   entry24    2   46   140   271   0.00      0      0
   entry42    2   49   119   258   0.00      0      0
   entry40    3   50    70   253   0.00      0      0
   entry15    4   50   112   253   0.00      0      0
    entry2    3   50   101   247   0.00      0      0
   entry48    6   50   103   236   0.00      0      0
   entry35    3   50    91   234   0.00      0      0
   entry18    6   50   103   225   0.00      0      0
   entry49    8   50   103   219   0.00      0      0
   entry45    6   49   112   210   0.00      0      0
   entry25   10   50   111   201   0.00      0      0
   entry11   10   50   106   199   0.00      0      0
   entry22   16   50   104   184   0.00      0      0
    entry5   13   50    84   184   0.00      0      0
   entry39   18   50   104   177   0.00      0      0
   entry44   27   50    91   152   0.00      0      0
```

We want to generate all possible outcomes and score all the entries
against them. Set up a DFS with inputs:
* games - array of game numbers remaining
* gamesLeft - size of this array
* game - game in games array under review
* stats - array of stats structs, one per entry
  * maxRank, minRank, maxScore, timesWon, timesTied, possibleScore, champCounts[], bracket
  * possibleScore is set to current bracket score given state of tournament so far

Algo:
1. Initial setup as above
1. Determine the two teams in games[game]
1. Assume winner is the first team, calculate gameScore for each entry, add
   to possibleScore = possibleScore + gameScore
1. Recurse with game += 1. If game == gamesLeft, sort stats by possiblScore and update stats for this result, stop recursion.
1. Subtract the gameScore from step 2 from each possibleScore of each entry
1. Repeat 2-5 with winner assumed to be second team

max recursion depth would be = number of games remaining

Parallelization of Possibilities Report
---------------------------------------
We can parallelize the above to a power of 2 processes/servers/cores/etc if the
current tournament bracket is advanced and the algo run from there, then results
collected and combined.

2 runners:
  * 1st runner assumes game 1 winner is team1
  * 2nd runner assumes game 1 winner is team2

4 runners:
  * 1st runner assumes game 1 winner is team1, game 2 winner is team1
  * 2nd runner assumes game 1 winner is team2, game 2 winner is team1
  * 3nd runner assumes game 1 winner is team1, game 2 winner is team2
  * 4th runner assumes game 1 winner is team2, game 2 winner is team2

8 runners:
  * 1st runner assumes game 1 winner is team1, game 2 winner is team1, game 3 winner is team1
  * 2nd runner assumes game 1 winner is team2, game 2 winner is team1, game 3 winner is team1
  * 3nd runner assumes game 1 winner is team1, game 2 winner is team2, game 3 winner is team1
  * 4th runner assumes game 1 winner is team2, game 2 winner is team2, game 3 winner is team1
  * 5th runner assumes game 1 winner is team1, game 2 winner is team1, game 3 winner is team2
  * 6th runner assumes game 1 winner is team2, game 2 winner is team1, game 3 winner is team2
  * 7th runner assumes game 1 winner is team1, game 2 winner is team2, game 3 winner is team2
  * 8th runner assumes game 1 winner is team2, game 2 winner is team2, game 3 winner is team2

etc

Example run with 8 processes, collecting results into binary files in the pool directory:

```console
./pool -d test/fifty_entries -b 0 -n 8 -f bin poss &
./pool -d test/fifty_entries -b 1 -n 8 -f bin poss &
./pool -d test/fifty_entries -b 2 -n 8 -f bin poss &
./pool -d test/fifty_entries -b 3 -n 8 -f bin poss &
./pool -d test/fifty_entries -b 4 -n 8 -f bin poss &
./pool -d test/fifty_entries -b 5 -n 8 -f bin poss &
./pool -d test/fifty_entries -b 6 -n 8 -f bin poss &
./pool -d test/fifty_entries -b 7 -n 8 -f bin poss &
```

When `-f bin` is used, a binary file with partial stats is written to the pool directory.
The partial stats file names are `poss_N_of_M.bin`.

After producing the files (possibly on different machines), collect them into the
pool directory, then generate the possibilities report:

```console
./pool -d test/fifty_entries -r poss
```

Using this method and a compute cloud of 8,192 machines, we can generate a possibilities
report after Day 1 is complete in less than 1 hour. LOL.

On a single machine, the best you are going to be able to do is generate the report
in less than 1 hour by running 8 processes once there are about 38 teams remaining.

Game Index
-----------
Game numbers by round and what game numbers teams will play in
as they advance. Read across and up.

```
                    Game indexes
                      Round
Seeds Teams     1  2   3   4   5  Championship
1-16  t1 t2:    0 32--48--56--60--62
8-9   t3 t4:    1--+   |   |   |   |
5-12  t5 t6:    2 33---+   |   |   |
4-13  t7 t8:    3--+       |   |   |
6-11  t9 t10:   4 34--49 --+   |   |
3-14  t11 t12:  5--+   |       |   |
7-10  t13 t14:  6 35---+       |   |
2-15  t15 t16:  7--+           |   |
1-16  t17 t18:  8-36--50--57---+   |
8-9   t19 t20:  9                  |
5-12  t21 t22: 10 37               |
4-13  t23 t24: 11                  |
6-11  t25 t26: 12 38  51           |
3-14  t27 t28: 13                  |
7-10  t29 t30: 14 39               |
2-15  t31 t32: 15                  |
                                   |
1-16  t33 t34: 16 40  52  58  61---+
8-9   t35 t36: 17
5-12  t37 t38: 18 41
4-13  t39 t40: 19
6-11  t41 t42: 20 42  53
3-14  t43 t44: 21
7-10  t45 t46: 22 43
2-15  t47 t48: 23
1-16  t49 t50: 24 44  54  59
8-9   t51 t52: 25
5-12  t53 t54: 26 45
4-13  t55 t56: 27
6-11  t57 t58: 28 46  55
3-14  t59 t60: 29
7-10  t61 t62: 30 47
2-15  t63 t64: 31
```
