Q3: alloc() and new() zero-initialise memory
Stage: Stage 6-7
Files changed: tnode.c
Test file(s): test_q03.txt
Expected: prints 0 (original compiler prints 99)

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what was changed relative to your original stage8/ex1.
