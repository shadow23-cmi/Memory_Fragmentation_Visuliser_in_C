# Memory_Fragmentation_Visuliser_in_C
A simple code to visualise memory fragmentation (both internal and external) for the following allocation algorithms: 1. first fit 2. best fit 3. worst fit 4. paging
Compilation:
  gcc -c BST.c
  gcc BST.o first_fit.c -o first_fit
  gcc BST.o best_fit.c -o best_fit
  gcc BST.o worst_fit.c -o worst_fit
  gcc BST.o paging.c -o paging
