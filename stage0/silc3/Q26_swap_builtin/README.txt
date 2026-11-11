Q26_swap_builtin: swap(a, b) statement for int lvalues
Stage: Stage 5-6
Files changed: expr.l, expr.y
Test file(s): test_q26.txt, test_q26_error.txt
Expected: 8 3 20 10 9 1 6 5 20 8 ; error file -> cannot swap a variable with itself
Note: Built purely in the grammar as three assignments (a=a+b; b=a-b; a=a-b), so codegen is untouched. Works on variables, array elements and record fields.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
