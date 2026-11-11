Q28_not_equal_alias: <> as another spelling of !=
Stage: Stage 4
Files changed: expr.l
Test file(s): test_q28.txt
Expected: input 3 3 -> 0 0 ; input 3 4 -> 1 1
Note: One lexer rule, no parser change.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
