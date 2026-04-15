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
#define FRAMESIZE 4
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_RESET "\x1b[0m"


int MEMORY[MAX_MEMORY_WIDTH][MAX_MEMORY_LENGTH];
int MEMSIZENEWPROCESS = 0;
int PROCESSBSTROOT = -1;
int TOTALMEM = MAX_MEMORY_LENGTH * MAX_MEMORY_WIDTH;
int NO_OF_FRAMES = (MAX_MEMORY_LENGTH * MAX_MEMORY_WIDTH)/FRAMESIZE;
int ALLOCATEDMEM = 0;
float INTERNAL_FRAGMENTATION = 0;

int FREE_FRAME_LIST[(MAX_MEMORY_LENGTH * MAX_MEMORY_WIDTH)/FRAMESIZE];
int NO_OF_FREE_FRAMES = (MAX_MEMORY_LENGTH * MAX_MEMORY_WIDTH)/FRAMESIZE;
int FREE_FRAME_LIST_START = 0;

typedef struct {
    int frame_no;
    int occupied;
} PTE;

typedef struct {
    int no_of_entries;
    PTE* table;
} PAGETABLE;

typedef struct {
    int PID;
    int size;
    PAGETABLE page_table;
} PROCESS;


int comparePID(void* p1, void* p2)
{
    /* TODO */
    PROCESS* process1 = (PROCESS*)p1;
    PROCESS* process2 = (PROCESS*)p2;
    if(process1->PID > process2->PID) return 1;
    else if(process1->PID < process2->PID) return -1;
    else return 0;
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

int InitFreeFrameList(void)
{
    for(int i=0;i<NO_OF_FRAMES-1;i++)
    {
        FREE_FRAME_LIST[i] = i+1;
    }
    FREE_FRAME_LIST[NO_OF_FRAMES-1] = -1;
    return 1;
}

int AllocateFreeFrames(PAGETABLE page_table, int size)
{
    int i = 0;
    int temp = FREE_FRAME_LIST_START;
    int frames_needed = page_table.no_of_entries;
    if (frames_needed > NO_OF_FREE_FRAMES) return 0;
    for (i=0;i<frames_needed-1;i++)
    {
        page_table.table[i].frame_no = temp;
        page_table.table[i].occupied = FRAMESIZE;

        temp = FREE_FRAME_LIST[temp];
    }
    page_table.table[frames_needed-1].frame_no = temp;
    int rem = size % FRAMESIZE;
    page_table.table[frames_needed-1].occupied = (rem == 0) ? FRAMESIZE : rem;

    if (FREE_FRAME_LIST_START == -1) return 0;
    FREE_FRAME_LIST_START = FREE_FRAME_LIST[temp];
    NO_OF_FREE_FRAMES -= frames_needed;

    return 1;
}

int DeAllocateFrames(PAGETABLE page_table)
{
    int i = 0;
    int frames_to_be_freed = page_table.no_of_entries;
    for (i=0;i<frames_to_be_freed;i++)
    {
        int temp = page_table.table[i].frame_no;
        FREE_FRAME_LIST[temp] = FREE_FRAME_LIST_START;
        FREE_FRAME_LIST_START = temp;
    }
    NO_OF_FREE_FRAMES += frames_to_be_freed;
    free(page_table.table);
    return 1;
}
/*
void AlloacteMemory(PAGETABLE page_table)
{
    for(int i=0;i<page_table.no_of_entries;i++)
    {
        for(int j =0;j<page_table.table[i].occupied;j++)
        {
            MEMORY[page_table.table[j].frame_no][j] = 1;
        }
    }
}
*/
void AlloacteMemory(PAGETABLE page_table, int PID)
{
    for (int i = 0; i < page_table.no_of_entries; i++)
    {
        int frame = page_table.table[i].frame_no;
        int bytes = page_table.table[i].occupied;

        for (int j = 0; j < bytes; j++)
        {
            int index = frame * FRAMESIZE + j;

            int row = index % MAX_MEMORY_WIDTH;
            int col = index / MAX_MEMORY_WIDTH;

            if (col < MAX_MEMORY_LENGTH)
                MEMORY[row][col] = PID;
        }
    }
}

void DeAlloacteMemory(PAGETABLE page_table)
{
    for (int i = 0; i < page_table.no_of_entries; i++)
    {
        int frame = page_table.table[i].frame_no;
        int bytes = page_table.table[i].occupied;

        for (int j = 0; j < bytes; j++)
        {
            int index = frame * FRAMESIZE + j;

            int row = index % MAX_MEMORY_WIDTH;
            int col = index / MAX_MEMORY_WIDTH;

            if (col < MAX_MEMORY_LENGTH)
                MEMORY[row][col] = -1;
        }
    }
}

int AllocatePID(BST* processes, int PID, int size)
{
    PROCESS process;
    PAGETABLE page_table;
    int frames_needed = (FRAMESIZE + size -1)/FRAMESIZE; // ceiling function

    page_table.no_of_entries = frames_needed;
    page_table.table = malloc(frames_needed*sizeof(PTE));

    if (!AllocateFreeFrames(page_table, size))
    {
        printf("Allocation   PID: %d size: %d MB ",PID, size);
        printf(ANSI_COLOR_RED "Paging Allocation Failed\n" ANSI_COLOR_RESET);
        free(page_table.table);
        return 0;
    }

    AlloacteMemory(page_table, PID);
    process.PID = PID,
    process.size = size;
    process.page_table = page_table;

    PROCESSBSTROOT = Insert(processes, PROCESSBSTROOT, &process);

    ALLOCATEDMEM += size;
    int rem = size % FRAMESIZE;
    int frag = (rem == 0) ? 0 : (FRAMESIZE - rem);
    INTERNAL_FRAGMENTATION += frag;

    printf("\nAllocated PID %d size %d using %d frames\n", PID, size, frames_needed);
    return 1;

}

int DeAllocatePID(BST* processes, int PID)
{
    PROCESS process;
    //PAGETABLE page_table;

    process.PID = PID;
    // search for the PID in processes list to get the start position in memory
    int pid_search_idx = Search(processes,PROCESSBSTROOT,&process);
    if (pid_search_idx == -1)
    {
        printf("PID not found\n");
        return 0;
    }
    process.page_table = ((PROCESS*)DATA_AT(processes,pid_search_idx))->page_table;
    process.size = ((PROCESS*)DATA_AT(processes,pid_search_idx))->size;

    DeAlloacteMemory(process.page_table);
    if (!DeAllocateFrames(process.page_table))
    {
        printf(ANSI_COLOR_RED "Paging DeAllocation Failed\n" ANSI_COLOR_RESET);
        free(process.page_table.table);
        return 0;
    }

    PROCESSBSTROOT = Delete(processes, PROCESSBSTROOT, &process);
    //free(process.page_table.table);
    ALLOCATEDMEM -= process.size;
    int rem = process.size % FRAMESIZE;
    int frag = (rem == 0) ? 0 : (FRAMESIZE - rem);
    INTERNAL_FRAGMENTATION -= frag;

    printf("DeAllocated PID %d size %d \n", PID, process.size);
    return 1;
}

void HighlightPID(int PID)
{
    system("clear");
    int row, col, free_mem = TOTALMEM - ALLOCATEDMEM;
    float fragmentation = ((float)INTERNAL_FRAGMENTATION / TOTALMEM) * 100.0;
    printf("\v\v");
    printf("\n Total Mmeory: %d MB, Memory in use: %d MB, Free Memory: %d MB\n", TOTALMEM, ALLOCATEDMEM, free_mem);
    printf(" No of Free Frames: %d , Fragmentation: %.2f %%\n", NO_OF_FREE_FRAMES, fragmentation);
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
                printf(ANSI_COLOR_MAGENTA "*" ANSI_COLOR_RESET );
            else
                printf(ANSI_COLOR_GREEN "*" ANSI_COLOR_RESET );
        }
        printf("\n");
    }
    printf("\v\v\n");
}

void Display(void)
{
    system("clear");
    int row, col, free_mem = TOTALMEM - ALLOCATEDMEM;
    float fragmentation = ((float)INTERNAL_FRAGMENTATION / TOTALMEM) * 100.0;
    printf("\v\v");
    printf("\n Total Mmeory: %d MB, Memory in use: %d MB, Free Memory: %d MB\n", TOTALMEM, ALLOCATEDMEM, free_mem);
    printf(" No of Free Frames: %d , Fragmentation: %.2f %%\n", NO_OF_FREE_FRAMES, fragmentation);
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
                printf(ANSI_COLOR_GREEN "*" ANSI_COLOR_RESET );
                //printf("*");
        }
        printf("\n");
    }
    printf("\v\v\n");
}

int Simulate(BST* processes, int num, float speed)
{
    PROCESS pid_to_free, temp;
    int pid, pid_to_free_id, size;

    srandom((int) time(NULL));
    for (int i = 0; i < num; i++)
    {
        pid = random() % 1000;
        size = (random() % (100 - 1 + 1)) + 1;
        temp.PID = pid;
        if (Search(processes, PROCESSBSTROOT, &temp) != -1) continue;  // PID already exists → generate a new one;
        
        if(!AllocatePID(processes, pid, size))return 0;
        else usleep((int)(600000/speed));
        printf("\n");
        Display();
        
        // randomely delete PID
        pid_to_free_id = RandomNodePicker(processes, PROCESSBSTROOT);
        if (pid_to_free_id != -1)
        {
            pid_to_free.PID = ((PROCESS*)DATA_AT(processes, pid_to_free_id))->PID;
            DeAllocatePID(processes, pid_to_free.PID);
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

int main(void)
{
    BST processes;
    PROCESS temp;
    int choice, pid, size, num, speed;

    InitBST(&processes, sizeof(PROCESS), MAX_PROCESSES, comparePID);
    InitMemoryAlloc();
    InitFreeFrameList();

    while (1)  // Infinite loop
    {
        printf("\n========== MEMORY MANAGEMENT MENU ==========\n");
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
                if (Search(&processes, PROCESSBSTROOT, &temp) != -1)  // PID already exists → generate a new one;
                {
                    Display();
                    printf(ANSI_COLOR_GREEN "PID already exist!!"ANSI_COLOR_RESET"!\n");
                    break;
                }
                if (!AllocatePID(&processes, pid, size))
                {
                    Display();
                    printf(ANSI_COLOR_RED "Allocation failed"ANSI_COLOR_RESET"!\n");
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

                if (!DeAllocatePID(&processes, pid))
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

                Simulate(&processes, num, speed);
                break;
            case 5:
                Display();
                DisplayPIDs(&processes);
                break;
            case 6:
                printf("Enter PID to Highlight: ");
                scanf("%d", &pid);
                HighlightPID(pid);
                printf("Highlighted portion belongs to PID: "ANSI_COLOR_MAGENTA"%d"ANSI_COLOR_RESET"\n",pid);
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
