Q12: this as alias of self, fi as alias of endif
Stage: Stage 7
Files changed: expr.l
Test file(s): test_q12.txt
Expected: 1

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
