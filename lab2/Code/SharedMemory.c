#include <stdio.h>
#include "includes.h"
#include <string.h>

// Define mutex semaphore
OS_EVENT *task0StateSemaphore;
OS_EVENT *task1StateSemaphore;

/* Definition of Task Stacks */
/* Stack grows from HIGH to LOW memory */
#define   TASK_STACKSIZE       2048
OS_STK    task0_stk[TASK_STACKSIZE];
OS_STK    task1_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define TASK0_PRIORITY      6  // highest priority
#define TASK1_PRIORITY      7

// Shared memory location
int sharedAddress = 0;

void task0(void* pdata)
{
    INT8U err;
    int number = 1; // Starting integer number
    
    while (1)
    {
        sharedAddress = number;
        printf("Sending : %d\n", sharedAddress);

        // Signal task1 to process the number
        OSSemPost(task0StateSemaphore);
        // Wait for task1 to finish processing
        OSSemPend(task1StateSemaphore, 0, &err);

        printf("Receiving : %d\n", sharedAddress);
        number++;
    }
}

void task1(void* pdata)
{
    INT8U err;
    while (1) // Keep running indefinitely
    { 
        // Wait for task0 to send a number
        OSSemPend(task0StateSemaphore, 0, &err);
        
        // Process the number
        sharedAddress *= -1;

        // Signal task0 that processing is done
        OSSemPost(task1StateSemaphore);
    }
}

/* The main function creates two task and starts multi-tasking */
int main(void)
{
    printf("Shared Memory Communication\n");

    // initialize semaphores
    task0StateSemaphore = OSSemCreate(0); 
    task1StateSemaphore = OSSemCreate(0); // Initially set to 0 so task1 waits for task0

    OSTaskCreateExt
    (
        task0,
        NULL,
        &task0_stk[TASK_STACKSIZE-1],
        TASK0_PRIORITY,
        TASK0_PRIORITY,
        &task0_stk[0],
        TASK_STACKSIZE,
        NULL,
        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
    );

    OSTaskCreateExt
    (
        task1,
        NULL,
        &task1_stk[TASK_STACKSIZE-1],
        TASK1_PRIORITY,
        TASK1_PRIORITY,
        &task1_stk[0],
        TASK_STACKSIZE,
        NULL,
        OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
    );  

    OSStart();
    return 0;
}
