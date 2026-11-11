Q14: break/continue outside a loop is an error
Stage: Stage 3-5
Files changed: tnode.c
Test file(s): test_q14_error.txt
Expected: Error: break outside a loop

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
