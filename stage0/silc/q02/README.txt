Q2: Disallow recursion (direct and mutual)
Stage: Stage 5
Files changed: symboltable.h, expr.y
Test file(s): test_q02_direct_error.txt, test_q02_mutual_error.txt, test_q02_ok.txt
Expected: first two -> compile error; ok -> 7

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
