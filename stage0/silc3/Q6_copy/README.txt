Q6_copy: copy(p) built-in (shallow copy)
Stage: Stage 6
Files changed: tnode.h, expr.l, expr.y, tnode.c
Test file(s): test_q21.txt, test_q21_type_error.txt
Expected: 3 10 4 71 10 71 5 ; type_error file -> copy() source and destination must have the same type
Note: Works for variables and record fields on the left side.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
