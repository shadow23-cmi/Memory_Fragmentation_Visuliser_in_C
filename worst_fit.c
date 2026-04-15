#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "BST.h"

#define MAX_PROCESSES 100
#define MAX_FREE_LOCATIONS 100
#define MAX_MEMORY_LENGTH 128
#define MAX_MEMORY_WIDTH 16
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_BLUE  "\x1b[34m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"

typedef struct {
    int PID;
    int start;
    int size;
} PROCESS;

typedef struct {
    int start;
    int size;
} FREE_LOCATION;


int MEMORY[MAX_MEMORY_WIDTH][MAX_MEMORY_LENGTH];
int MEMSIZENEWPROCESS = 0;
int PROCESSBSTROOT = -1;
int FREEMEMBSTROOT = -1;
int FREEMEMSIZEBSTROOT = -1;
int TOTALMEM = MAX_MEMORY_LENGTH * MAX_MEMORY_WIDTH;
int LARGESTFREEBLOCK = MAX_MEMORY_LENGTH * MAX_MEMORY_WIDTH;
int ALLOCATEDMEM = 0;


int comparePID(void* p1, void* p2)
{
    /* TODO */
    PROCESS* process1 = (PROCESS*)p1;
    PROCESS* process2 = (PROCESS*)p2;
    if(process1->PID > process2->PID) return 1;
    else if(process1->PID < process2->PID) return -1;
    else return 0;
}

int compareFreeMemFirstFit(void* f1, void* f2)
{
    FREE_LOCATION* a = (FREE_LOCATION*)f1;
    FREE_LOCATION* b = (FREE_LOCATION*)f2;

    if (a->start > b->start) return 1;
    else if (a->start < b->start) return -1;
    else return 0;
}

int compareFreeMemWorstFitBestFit(void* f1, void* f2)
{
    FREE_LOCATION* a = f1;
    FREE_LOCATION* b = f2;

    if (a->size != b->size)
        return a->size - b->size;

    return a->start - b->start; // tie-breaker
}


void InitMemoryAlloc(void)
{
    int row, col;

    //memory
    for(row=0;row<MAX_MEMORY_WIDTH;row++)
    {
        for(col=0;col<MAX_MEMORY_LENGTH;col++)
        {
            MEMORY[row][col] = -1; // -1 signifies free, intially all of memory free
        }
    }
    return;
}

void AllocateMmeory(int PID, int start, int size)
{
    for (int i = 0; i < size; i++)
    {
        int index = start + i;

        int row = index % MAX_MEMORY_WIDTH;
        int col = index / MAX_MEMORY_WIDTH;

        if (col < MAX_MEMORY_LENGTH)
            MEMORY[row][col] = PID;
    }
}

void DeAllocateMmeory(int start, int size)
{
    for (int i = 0; i < size; i++)
    {
        int index = start + i;

        int row = index % MAX_MEMORY_WIDTH;
        int col = index / MAX_MEMORY_WIDTH;

        if (col < MAX_MEMORY_LENGTH)
            MEMORY[row][col] = -1;
    }
}

int FindLargestFreeMemory(BST* free_mem_by_size)
{
    int idx = FindMax(free_mem_by_size, FREEMEMSIZEBSTROOT);
    LARGESTFREEBLOCK = ((FREE_LOCATION*)DATA_AT(free_mem_by_size, idx))->size;
    return 0;
}

int AllocatePID(BST* processes, BST* free_mem, BST* free_mem_by_size, int PID, int size)
{
    PROCESS process;

    int idx = FindMax(free_mem_by_size, FREEMEMSIZEBSTROOT);
    if ((idx == -1) || ((FREE_LOCATION*)DATA_AT(free_mem_by_size, idx))->size < size)
    {
        printf("Allocation   PID: %d size: %d MB ",PID, size);
        printf(ANSI_COLOR_RED "Allocation failed" ANSI_COLOR_RESET "\n");
        return 0;
    }

    FREE_LOCATION* loc = (FREE_LOCATION*)DATA_AT(free_mem_by_size, idx);

    int start = loc->start;
    int old_size = loc->size;

    // Remove old block
    FREEMEMBSTROOT  = Delete(free_mem, FREEMEMBSTROOT, loc);
    FREEMEMSIZEBSTROOT  = Delete(free_mem_by_size, FREEMEMSIZEBSTROOT, loc);

    // Insert remaining block if any
    if (old_size > size)
    {
        FREE_LOCATION newloc;
        newloc.start = start + size;
        newloc.size = old_size - size;

        FREEMEMBSTROOT = Insert(free_mem, FREEMEMBSTROOT, &newloc);
        FREEMEMSIZEBSTROOT = Insert(free_mem_by_size, FREEMEMSIZEBSTROOT, &newloc);
    }

    // Create process
    process.PID = PID;
    process.size = size;
    process.start = start;

    PROCESSBSTROOT = Insert(processes, PROCESSBSTROOT, &process);

    //actual allocation in simulated physical memory happens here
    AllocateMmeory(PID, start, size);
    
    ALLOCATEDMEM += size;
    printf("Allocated at Mem Position: %d\n", start);
    FindLargestFreeMemory(free_mem_by_size); // keeping track of largest free memory block for fragmentation calculation

    return 1;
}

int DeAllocatePID(BST* processes, BST* free_mem, BST* free_mem_by_size, int PID)
{
    PROCESS process;
    FREE_LOCATION freed_mem, next_free_mem, prev_free_mem;
    //int start;

    process.PID = PID;
    process.start = -1; //dummy value, any value would work

    // search for the PID in processes list to get the start position in memory
    int pid_search_idx = Search(processes,PROCESSBSTROOT,&process);
    if (pid_search_idx == -1)
    {
        printf("PID not found\n");
        return 0;
    }
    process.start = ((PROCESS*)DATA_AT(processes,pid_search_idx))->start;
    process.size = ((PROCESS*)DATA_AT(processes,pid_search_idx))->size;
    // Then deleting the PID from processes list
    PROCESSBSTROOT = Delete(processes,PROCESSBSTROOT,&process);

    // adding this freed memory to free memory list
    freed_mem.size = process.size;
    freed_mem.start = process.start;

    /*  
        Merge consecutive memory if any 
        i.e check if previous free mem ends at current freed PID mem
        & next free memory start at the end of current freed PID mem
    */
    // check if contagious with next free memory 
    next_free_mem.start = process.start+process.size;
    int next_free_mem_search_idx = Search(free_mem,FREEMEMBSTROOT,&next_free_mem);
    if(next_free_mem_search_idx != -1)
    {
        next_free_mem.size = ((FREE_LOCATION*)DATA_AT(free_mem,next_free_mem_search_idx))->size;
        FREEMEMBSTROOT  = Delete(free_mem, FREEMEMBSTROOT, &next_free_mem);
        FREEMEMSIZEBSTROOT  = Delete(free_mem_by_size, FREEMEMSIZEBSTROOT, &next_free_mem);
        freed_mem.size += next_free_mem.size; // append the next free mem to current free mem
    }
    // check if contagious with previous free memory 
    int prev_free_mem_search_idx = SearchMaxLessThan(free_mem,FREEMEMBSTROOT,&freed_mem);
    if(prev_free_mem_search_idx != -1)
    {
        prev_free_mem.size = ((FREE_LOCATION*)DATA_AT(free_mem,prev_free_mem_search_idx))->size;
        prev_free_mem.start = ((FREE_LOCATION*)DATA_AT(free_mem,prev_free_mem_search_idx))->start;
        if(prev_free_mem.start+prev_free_mem.size == freed_mem.start)
        {
            freed_mem.start = prev_free_mem.start;// the current free mem start from start of prev free mem
            freed_mem.size += prev_free_mem.size; // append the prev free mem to current free mem
            FREEMEMBSTROOT  = Delete(free_mem, FREEMEMBSTROOT, &prev_free_mem);
            FREEMEMSIZEBSTROOT  = Delete(free_mem_by_size, FREEMEMSIZEBSTROOT, &prev_free_mem);
        }
    }
    // insert the current free mem to free mem list
    FREEMEMBSTROOT  = Insert(free_mem, FREEMEMBSTROOT, &freed_mem);
    FREEMEMSIZEBSTROOT  = Insert(free_mem_by_size, FREEMEMSIZEBSTROOT, &freed_mem);
    // actual deallocation from simulated RAM
    DeAllocateMmeory(process.start, process.size);
    ALLOCATEDMEM -= process.size;
    FindLargestFreeMemory(free_mem_by_size); // keeping track of largest free memory block for fragmentation calculation

    return 1;
}

void Display(void)
{
    system("clear");
    int row, col, free_mem = TOTALMEM - ALLOCATEDMEM;
    float fragmentation = (1.0-((float)LARGESTFREEBLOCK/free_mem))*100.0;
    printf("\v\v");
    printf("\n Total Mmeory: %d MB, Memory in use: %d MB, Free Memory: %d MB\n", TOTALMEM, ALLOCATEDMEM, free_mem);
    printf(" Largest Free Block: %d MB, Fragmentation: %f %%\n", LARGESTFREEBLOCK, fragmentation);
    printf("\v\v\n");
    for(row=0;row<MAX_MEMORY_WIDTH;row++)
    {
        printf("\t\t");
        for(col=0;col<MAX_MEMORY_LENGTH;col++)
        {
            if(MEMORY[row][col] == -1)
                printf("*");
                //printf(ANSI_COLOR_GREEN "*" ANSI_COLOR_RESET );
            else if(MEMORY[row][col] >= 0)
                printf(ANSI_COLOR_BLUE "*" ANSI_COLOR_RESET );
                //printf("*");
        }
        printf("\n");
    }
    printf("\v\v\n");
}

void HighlightPID(int PID)
{
    system("clear");
    int row, col, free_mem = TOTALMEM - ALLOCATEDMEM;
    float fragmentation = (1.0-((float)LARGESTFREEBLOCK/free_mem))*100.0;
    printf("\v\v");
    printf("\n Total Mmeory: %d MB, Memory in use: %d MB, Free Memory: %d MB\n", TOTALMEM, ALLOCATEDMEM, free_mem);
    printf(" Largest Free Block: %d MB, Fragmentation: %f %%\n", LARGESTFREEBLOCK, fragmentation);
    printf("\v\v\n");
    for(row=0;row<MAX_MEMORY_WIDTH;row++)
    {
        printf("\t\t");
        for(col=0;col<MAX_MEMORY_LENGTH;col++)
        {
            if(MEMORY[row][col] == -1)
                printf("*");
                //printf(ANSI_COLOR_GREEN "*" ANSI_COLOR_RESET );
            else if(MEMORY[row][col] == PID)
                printf(ANSI_COLOR_YELLOW "*" ANSI_COLOR_RESET );
            else
                printf(ANSI_COLOR_BLUE "*" ANSI_COLOR_RESET );
                //printf("*");
        }
        printf("\n");
    }
    printf("\v\v\n");
}

int Simulate(BST* processes, BST* free_mem, BST* free_mem_by_size, int num, float speed)
{
    PROCESS temp;
    int pid, pid_to_free_id, size;

    srandom((int) time(NULL));
    for (int i = 0; i < num; i++)
    {
        pid = random() % 1000;
        size = (random() % (100 - 1 + 1)) + 1;
        temp.PID = pid;
        temp.size = size;
        if (Search(processes, PROCESSBSTROOT, &temp) != -1) continue;  // PID already exists → generate a new one;
        
        
        if(!AllocatePID(processes, free_mem, free_mem_by_size, pid, size))return 0;
        else usleep((int)(600000/speed));
        printf("\n");
        Display();
        
        // randomely delete PID
        pid_to_free_id = RandomNodePicker(processes, PROCESSBSTROOT);
        if (pid_to_free_id != -1)
        {
            temp.PID = ((PROCESS*)DATA_AT(processes, pid_to_free_id))->PID;
            //temp.size = ((PROCESS*)DATA_AT(processes, pid_to_free_id))->size;
            temp.start = ((PROCESS*)DATA_AT(processes, pid_to_free_id))->start;
            DeAllocatePID(processes, free_mem, free_mem_by_size, temp.PID);
            printf("\n");
            Display();
            printf("Deallocation PID: %d size: %d MB\n",pid, size);
        }
        usleep((int)(600000/speed));  // 1,000,000 microseconds = 1 second

    }
    return 1;
}

void DisplayPIDs(BST* processes)
{
    PROCESS* list_of_processes = (PROCESS*)InorderIterative(processes,PROCESSBSTROOT);
    printf("\v\n");
    printf(ANSI_COLOR_GREEN"PIDs: "ANSI_COLOR_RESET);
    for(int i=0;i<processes->no_elements;i++)
    {
        printf("%d(%d MB) || ",list_of_processes[i].PID,list_of_processes[i].size);
    }
    printf("\v\n");
    free(list_of_processes);
}

/*
int main(void)
{
    //PROCESS* processes = NULL;
    BST processes;
    BST free_mem, free_mem_by_size;
    PROCESS pid_to_free;
    int num = 1000, pid, pid_to_free_id, size;
    

    InitBST(&processes, sizeof(PROCESS), MAX_PROCESSES, comparePID);
    InitBST(&free_mem,sizeof(FREE_LOCATION),MAX_FREE_LOCATIONS, compareFreeMemFirstFit);//firstfit mem allocation
    InitBST(&free_mem_by_size,sizeof(FREE_LOCATION),MAX_FREE_LOCATIONS, compareFreeMemWorstFitBestFit); // map to find largest free mem
    
    FREE_LOCATION initial;
    initial.start = 0;
    initial.size = MAX_MEMORY_WIDTH * MAX_MEMORY_LENGTH;
    FREEMEMBSTROOT = Insert(&free_mem, FREEMEMBSTROOT, &initial);
    FREEMEMSIZEBSTROOT = Insert(&free_mem_by_size, FREEMEMSIZEBSTROOT, &initial);

    InitMemoryAlloc();
    srandom((int) time(NULL));
    for (int i = 0; i < num; i++) {
        pid = random() % 1000;
        size = (random() % (100 - 1 + 1)) + 1;
        printf("Allocation   PID: %d size: %d MB ",pid, size);

        if(!AllocatePID(&processes,&free_mem, &free_mem_by_size, pid, size)) break;
        printf("\n");
        Display();
        usleep(400000);  // 1,000,000 microseconds = 1 second
 
        // randomely delete PID
        pid_to_free_id = RandomNodePicker(&processes, PROCESSBSTROOT);
        if (pid_to_free_id != -1)
        {
            pid_to_free.PID = ((PROCESS*)DATA_AT(&processes, pid_to_free_id))->PID;
            pid_to_free.size = ((PROCESS*)DATA_AT(&processes, pid_to_free_id))->size;
            DeAllocatePID(&processes, &free_mem, &free_mem_by_size, pid_to_free.PID);
            printf("\n");
            Display();
            printf("Deallocation PID: %d size: %d MB\n",pid, size);
        }

    }
    printf("\n");
    //Display();

    return 0;
}
*/

int main(void)
{
    //PROCESS* processes = NULL;
    BST processes;
    BST free_mem, free_mem_by_size;
    PROCESS temp;
    int pid, size, choice, num, speed;
    

    InitBST(&processes, sizeof(PROCESS), MAX_PROCESSES, comparePID);
    InitBST(&free_mem,sizeof(FREE_LOCATION),MAX_FREE_LOCATIONS, compareFreeMemFirstFit);//firstfit mem allocation
    InitBST(&free_mem_by_size,sizeof(FREE_LOCATION),MAX_FREE_LOCATIONS, compareFreeMemWorstFitBestFit); // map to find largest free mem
    //initially allocate the whole memory as a single free block
    FREE_LOCATION initial;
    initial.start = 0;
    initial.size = MAX_MEMORY_WIDTH * MAX_MEMORY_LENGTH;
    FREEMEMBSTROOT = Insert(&free_mem, FREEMEMBSTROOT, &initial);
    FREEMEMSIZEBSTROOT = Insert(&free_mem_by_size, FREEMEMSIZEBSTROOT, &initial);

    InitMemoryAlloc();

    Display();
    printf("\nInitially all memory free");
    while (1)  // Infinite loop
    {
        printf("\n========== "ANSI_COLOR_GREEN"MEMORY MANAGEMENT MENU ("ANSI_COLOR_CYAN"Worst Fit"ANSI_COLOR_RESET")==========\n");
        printf("1. Allocate Process\n");
        printf("2. Deallocate Process\n");
        printf("3. Display Memory\n");
        printf("4. Run Simulation\n");
        printf("5. List PIDs\n");
        printf("6. Highlight PID\n");
        printf("7. Exit\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1)
        {
            Display();
            printf(ANSI_COLOR_RED "Invalid input! Please enter a number"ANSI_COLOR_RESET".\n");
            // Clear input buffer
            while (getchar() != '\n');
            continue;
        }

        switch (choice)
        {
            case 1:
                printf("Enter PID: ");
                scanf("%d", &pid);
                printf("Enter size (MB): ");
                scanf("%d", &size);
                temp.PID = pid;
                temp.size = size;
                if (Search(&processes, PROCESSBSTROOT, &temp) != -1)  // PID already exists → generate a new one;
                {
                    Display();
                    printf(ANSI_COLOR_GREEN "PID already exist!!"ANSI_COLOR_RESET"!\n");
                    break;
                }
                if (!AllocatePID(&processes,&free_mem, &free_mem_by_size, pid, size))
                {
                    Display();
                }
                else
                {
                    Display();
                    printf(ANSI_COLOR_GREEN"Allocated  " ANSI_COLOR_RESET"PID: %d size: %d MB \n",pid, size);
                }
                break;

            case 2:
                printf("Enter PID to deallocate: ");
                scanf("%d", &pid);
                temp.PID = pid;
                //temp.size = size;  dangerous
                if (!DeAllocatePID(&processes, &free_mem, &free_mem_by_size, temp.PID))
                {
                    Display();
                    printf(ANSI_COLOR_RED "DeAllocation failed"ANSI_COLOR_RESET"!\n");
                }
                else
                    Display();
                break;

            case 3:
                Display();
                break;

            case 4:
                printf("Enter number of operations: ");
                scanf("%d", &num);
                printf("Enter speed (e.g. 0.1–10): ");
                scanf("%d", &speed);

                Simulate(&processes, &free_mem, &free_mem_by_size, num, speed);
                break;
            case 5:
                Display();
                DisplayPIDs(&processes);
                break;
            case 6:
                printf("Enter PID to Highlight: ");
                scanf("%d", &pid);
                HighlightPID(pid);
                printf("Highlighted portion belongs to PID: "ANSI_COLOR_YELLOW" %d"ANSI_COLOR_RESET"\n",pid);
                break;
            case 7:
                Display();
                printf(ANSI_COLOR_GREEN"Exiting...\n"ANSI_COLOR_RESET);
                exit(0);

            default:
                Display();
                printf(ANSI_COLOR_RED "Invalid choice"ANSI_COLOR_RESET"!\n");
        }
    }
    return 0;
}

