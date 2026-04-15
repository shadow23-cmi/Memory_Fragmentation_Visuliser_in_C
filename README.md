# Memory_Fragmentation_Visuliser_in_C
A simple code to visualise memory fragmentation (both internal and external) for the following allocation algorithms: 
  1. first fit
  2. best fit
  3. worst fit
  4. paging

## Compilation:
```c
  gcc -c lib/BST.c -o obj/BST.o
  gcc obj/BST.o src/first_fit.c -Ilib -o bin/first_fit
  gcc obj/BST.o src/best_fit.c  -Ilib -o bin/best_fit
  gcc obj/BST.o src/worst_fit.c -Ilib -o bin/worst_fit
  gcc obj/BST.o src/paging.c    -Ilib -o bin/paging

