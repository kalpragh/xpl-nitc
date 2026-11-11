Q4_short_circuit_logical: &&, ||, ! with short-circuit evaluation
Stage: Stage 4-5
Files changed: expr.l, expr.y, tnode.h, tnode.c
Test file(s): test_q19.txt
Expected: input 9 -> 0 200 1 300 600 700 900 ; input 0 -> 0 200 1 300 500 800 900
Note: The printed 0 (not 0 then 5) and 1 (not 1 then 7) prove the right operand is skipped; x=0 never divides by zero. Replaces the non-short-circuit version from the first zip.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
