Q10: Unary minus
Stage: Stage 5
Files changed: expr.y
Test file(s): test_q10.txt
Expected: 6 12 7 6

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
