Q2_compound_assignment: Compound assignment += -= *= /=
Stage: Stage 5
Files changed: expr.l, expr.y
Test file(s): test_q18.txt
Expected: input 7 4 -> 11 9 36 12 22 12 15
Note: Works on int variables, array elements and record fields (shared helper updateStmt + IncTarget rule).

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
