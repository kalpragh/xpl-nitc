Q24_len_builtin: len(array) built-in
Stage: Stage 4
Files changed: expr.l, expr.y
Test file(s): test_q24.txt, test_q24_error.txt
Expected: 7 42 3 ; error file -> len() needs a global array
Note: Resolved at compile time into a constant node: elements for 1D, rows for 2D.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
