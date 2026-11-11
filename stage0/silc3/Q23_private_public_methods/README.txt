Q23_private_public_methods: private / public access for class methods
Stage: Stage 7
Files changed: classtable.h, classtable.c, expr.l, expr.y, tnode.c
Test file(s): test_q23.txt, test_q23_outside_error.txt, test_q23_subclass_error.txt
Expected: 10 24 24 ; both error files -> method 'audit' is private to class 'Account'
Note: Methods are public by default. A private method can only be called through self from methods of the class that declared it (not from main, other objects, or subclasses). Complements the earlier q04 (public fields).

Build:   bash build.sh        (needs lex, yacc, gcc)
Compile: ./compiler <testfile>      (writes output.xsm)
Label:   ./label output.xsm final.xsm
Then load final.xsm and library.lib with your usual xsm / xfs-interface steps.
changes.diff shows exactly what changed relative to your original stage8/ex1.
