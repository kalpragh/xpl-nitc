# Extra ExpL modification solutions (questions not covered in the first zip)

Each folder is a full copy of your stage8/ex1 sources with only that change applied; each was built and run on its own tests.

Already covered in the first zip: Q1 (q01 + q10), Q3 (q08), Q8 (q05).

| Folder | Modification | Files changed | Expected |
|---|---|---|---|
| Q2_compound_assignment | Compound assignment += -= *= /= | expr.l, expr.y | input 7 4 -> 11 9 36 12 22 12 15 |
| Q4_short_circuit_logical | &&, ||, ! with short-circuit evaluation | expr.l, expr.y, tnode.h, tnode.c | input 9 -> 0 200 1 300 600 700 900 ; input 0 -> 0 200 1 300 500 800 900 |
| Q5_switch | switch / case / default / endswitch | tnode.h, expr.l, expr.y, tnode.c | 11 22 99 1115 ; duplicate file -> Error: duplicate case 1 in switch |
| Q6_copy | copy(p) built-in (shallow copy) | tnode.h, expr.l, expr.y, tnode.c | 3 10 4 71 10 71 5 ; type_error file -> copy() source and destination must have the same type |
| Q7_incdec_lvalues | ++ and -- on variables, array elements, record fields and self.field | expr.l, expr.y | 6 4 1 10 3 2 ; error file -> Type Mismatch (not an int lvalue) |
