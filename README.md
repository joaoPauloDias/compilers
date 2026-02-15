# Toy language compiler

This compiler was done during the Compilers course at INF/UFRGS, taught by professor Lucas Schnorr. It uses FLEX/BISON and is able to generate executables, albeit not optmized ones.

Instead of using the regular malloc/free for memory management, we used an arena allocator, which greatly reduced the amount of pointer juggling we had to do. We also kept the symbol table, AST and parsing offensively simple, which allowed us to get fewer bugs and fix them easily if they happened.
