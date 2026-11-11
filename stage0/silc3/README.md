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
| Q23_private_public_methods | private / public access for class methods | classtable.h, classtable.c, expr.l, expr.y, tnode.c | 10 24 24 ; both error files -> method 'audit' is private to class 'Account' |
| Q24_len_builtin | len(array) built-in | expr.l, expr.y | 7 42 3 ; error file -> len() needs a global array |
| Q25_abs_builtin | abs(expr) built-in | tnode.h, expr.l, expr.y, tnode.c | input -7 3 -> 7 3 10 22 ; input 4 -6 -> 4 6 10 25 |
| Q26_swap_builtin | swap(a, b) statement for int lvalues | expr.l, expr.y | 8 3 20 10 9 1 6 5 20 8 ; error file -> cannot swap a variable with itself |
| Q27_abstract_methods | abstract methods and abstract classes | symboltable.h, classtable.h, classtable.c, expr.l, expr.y, tnode.c | 12 24 20 ; error files -> cannot create an object of class ... abstract method 'area' is not implemented |
| Q28_not_equal_alias | <> as another spelling of != | expr.l | input 3 3 -> 0 0 ; input 3 4 -> 1 1 |
