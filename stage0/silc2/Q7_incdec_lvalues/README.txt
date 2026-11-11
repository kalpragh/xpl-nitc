Q7_incdec_lvalues: ++ and -- on variables, array elements, record fields and self.field
Stage: Stage 7
Files changed: expr.l, expr.y
Test file(s): test_q22.txt, test_q22_error.txt
Expected: 6 4 1 10 3 2 ; error file -> Type Mismatch (not an int lvalue)
Note: Superset of the earlier x++/x-- solution.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
