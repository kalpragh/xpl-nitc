Q15: At most 3 function parameters
Stage: Stage 5
Files changed: symboltable.c
Test file(s): test_q15_error.txt
Expected: Error: a function can have at most 3 parameters

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
