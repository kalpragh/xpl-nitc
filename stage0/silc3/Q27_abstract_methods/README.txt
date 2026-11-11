Q27_abstract_methods: abstract methods and abstract classes
Stage: Stage 8
Files changed: symboltable.h, classtable.h, classtable.c, expr.l, expr.y, tnode.c
Test file(s): test_q27.txt, test_q27_new_abstract_error.txt, test_q27_unimplemented_error.txt
Expected: 12 24 20 ; error files -> cannot create an object of class ... abstract method 'area' is not implemented
Note: An abstract method has no body and no vtable entry until a subclass implements it; new() on a class that still has an abstract method is an error.

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
