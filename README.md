Tournament 3
-------------
Back to basics NCAA basketball pool software implemented in pure C for speed.
No nonsensical bit twiddling.  2.7M bracket scores per second. Implemented
as a single file header for command line pool management.

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
