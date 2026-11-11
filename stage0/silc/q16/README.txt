Q16: Constant folding
Stage: Stage 4-5
Files changed: tnode.c
Test file(s): test_q16.txt
Expected: 14 8 (output.xsm has MOV R0, 14)

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
