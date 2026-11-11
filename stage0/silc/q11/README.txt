Q11: free(p) also sets p to null
Stage: Stage 6
Files changed: tnode.c
Test file(s): test_q11.txt
Expected: 1 1 0 (original compiler prints 0 0 0)

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
