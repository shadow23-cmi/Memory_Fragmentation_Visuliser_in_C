# Memory_Fragmentation_Visuliser_in_C
## Author:
  1. Madhumita Das
  2. Suman Polley
A simple code to visualise **Memory Fragmentation** (both internal and external) for the following Memory allocation algorithms: 
  1. **first fit**
  2. **best fit**
  3. **worst fit**
  4. **paging**

## Compilation:
```c
  gcc -c lib/BST.c -o obj/BST.o
  gcc obj/BST.o src/first_fit.c -Ilib -o bin/first_fit
  gcc obj/BST.o src/best_fit.c  -Ilib -o bin/best_fit
  gcc obj/BST.o src/worst_fit.c -Ilib -o bin/worst_fit
  gcc obj/BST.o src/paging.c    -Ilib -o bin/paging
```
## Ececution:
**N.B** for Best experience run in full screen
#### External Fragmentation:
##### for visulising first fit memory allocation allocation algorithm
```c
  bin/first_fit
```
##### for visulising best fit memory allocation allocation algorithm
```c
  bin/best_fit
```
##### for visulising worst fit memory allocation allocation algorithm
```c
  bin/worst_fit
```
#### Internal Fragmentation:
##### for visulising paging memory allocation allocation algorithm
```c
  bin/paging
```
## Folder Description:
#### lib
Contains the Binary Search Tree library
#### obj
Contains BST.o i.e the result of
```c
  gcc -c lib/BST.c -o obj/BST.o
```
#### src
Contains the .c files 
#### bin
Contains the binary executible files i.e the results of
```c
  gcc obj/BST.o src/first_fit.c -Ilib -o bin/first_fit
  gcc obj/BST.o src/best_fit.c  -Ilib -o bin/best_fit
  gcc obj/BST.o src/worst_fit.c -Ilib -o bin/worst_fit
  gcc obj/BST.o src/paging.c    -Ilib -o bin/paging
```

