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


void task0(void* pdata)
{
    INT8U err;
    int iterations = 0;
    while (iterations < 10)
    {
        
        OSSemPend(task1StateSemaphore, 0, &err);
        printf("Task 0 - State 0\n");
        OSSemPost(task0StateSemaphore);
        OSSemPend(task1StateSemaphore, 0, &err);
         printf("Task 0 - State 1\n");
        OSSemPost(task1StateSemaphore);
        iterations++;
    }
}

void task1(void* pdata)
{
    INT8U err;
    int iterations = 0;
    
    while (iterations < 10)
    { 
        OSSemPend(task0StateSemaphore, 0, &err);
        printf("Task 1 - State 0\n");
        printf("Task 1 - State 1\n");
        OSSemPost(task1StateSemaphore);
       
        iterations++;
    }
}

/* The main function creates two task and starts multi-tasking */
int main(void)
{
    printf("Lab 3 - Handshake\n");

    // initialize semaphores
    task0StateSemaphore = OSSemCreate(0); 
    task1StateSemaphore = OSSemCreate(1); 

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
