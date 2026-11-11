Q25_abs_builtin: abs(expr) built-in
Stage: Stage 5
Files changed: tnode.h, expr.l, expr.y, tnode.c
Test file(s): test_q25.txt
Expected: input -7 3 -> 7 3 10 22 ; input 4 -6 -> 4 6 10 25
Note: Negates the value only when it is below 0.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
