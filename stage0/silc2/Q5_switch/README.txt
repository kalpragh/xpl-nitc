Q5_switch: switch / case / default / endswitch
Stage: Stage 5
Files changed: tnode.h, expr.l, expr.y, tnode.c
Test file(s): test_q20.txt, test_q20_duplicate_error.txt
Expected: 11 22 99 1115 ; duplicate file -> Error: duplicate case 1 in switch
Note: No fall-through; break leaves the switch; continue targets the enclosing loop; selector must be int; works inside functions.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
