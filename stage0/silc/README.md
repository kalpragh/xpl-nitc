# ExpL compiler modification solutions

Each folder qNN is a full copy of your stage8/ex1 sources with ONLY that question's change applied.
Every folder was built and run on its own test file.

Per folder: changed sources, build.sh, test_*.txt, changes.diff, README.txt.

| Q | Modification | Stage | Files changed | Expected result |
|---|---|---|---|---|
| Q1 | Modulo operator % | Stage 5 | expr.y, tnode.c | run with input: 17 5  ->  2 6 |
| Q2 | Disallow recursion (direct and mutual) | Stage 5 | symboltable.h, expr.y | first two -> compile error; ok -> 7 |
| Q3 | alloc() and new() zero-initialise memory | Stage 6-7 | tnode.c | prints 0 (original compiler prints 99) |
| Q4 | public fields in classes | Stage 7 | typetable.h, expr.l, expr.y, classtable.c, tnode.c | 12 7 ; error file -> private field error |
| Q5 | super.method(args) | Stage 8 | tnode.h, expr.l, expr.y, tnode.c | 13 43 13 |
| Q6 | final methods | Stage 8 | classtable.h, classtable.c, expr.l, expr.y | 1 20 ; error file -> cannot override final |
| Q7 | x++ and x-- statements | Stage 5 | expr.l, expr.y | 4 6 4 |
| Q8 | for loop (continue runs the update) | Stage 5-6 | expr.l, expr.y, tnode.c | 13 6 |
| Q9 | instanceof(var, Class) | Stage 8 | tnode.h, classtable.h, classtable.c, expr.l, expr.y, tnode.c | 1 1 0 0 100 |
| Q10 | Unary minus | Stage 5 | expr.y | 6 12 7 6 |
| Q11 | free(p) also sets p to null | Stage 6 | tnode.c | 1 1 0 (original compiler prints 0 0 0) |
| Q12 | this as alias of self, fi as alias of endif | Stage 7 | expr.l | 1 |
| Q13 | Compile-time division by zero error | Stage 4-5 | expr.y | Error: division by zero |
| Q14 | break/continue outside a loop is an error | Stage 3-5 | tnode.c | Error: break outside a loop |
| Q15 | At most 3 function parameters | Stage 5 | symboltable.c | Error: a function can have at most 3 parameters |
| Q16 | Constant folding | Stage 4-5 | tnode.c | 14 8 (output.xsm has MOV R0, 14) |
| Q17 | && and || operators | Stage 3-5 | expr.l, expr.y, tnode.h, tnode.c | input 3 4 -> 1 11 20 ; input 1 0 -> 0 11 21 |
