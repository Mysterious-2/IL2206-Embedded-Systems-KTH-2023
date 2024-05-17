#include <stdio.h>
#include "includes.h"
#include <string.h>

#define DEBUG 1

#define   TASK_STACKSIZE       2048
OS_STK    task1_stk[TASK_STACKSIZE];
OS_STK    task2_stk[TASK_STACKSIZE];
OS_STK    stat_stk[TASK_STACKSIZE];

#define TASK1_PRIORITY      6
#define TASK2_PRIORITY      7
#define TASK_STAT_PRIORITY 12

OS_EVENT *printSem; // Semaphore declaration

void printStackSize(char* name, INT8U prio) 
{
  INT8U err;
  OS_STK_DATA stk_data;
  err = OSTaskStkChk(prio, &stk_data);
  if (err == OS_NO_ERR) {
    if (DEBUG == 1)
      printf("%s (priority %d) - Used: %d; Free: %d\n", name, prio, stk_data.OSUsed, stk_data.OSFree);
  }
  else
    {
      if (DEBUG == 1)
	printf("Stack Check Error!\n");    
    }
}

void task1(void* pdata)
{
  while (1)
    { 
      INT8U err;
      OSSemPend(printSem, 0, &err); // Acquire semaphore before printing
      char text1[] = "Hello from Task1\n";
      int i;
      for (i = 0; i < strlen(text1); i++)
	    putchar(text1[i]);
      OSSemPost(printSem); // Release semaphore after printing is done
      OSTimeDlyHMSM(0, 0, 0, 11);
    }
}

void task2(void* pdata)
{
  while (1)
    { 
      INT8U err;
      OSSemPend(printSem, 0, &err); // Acquire semaphore before printing
      char text2[] = "Hello from Task2\n";
      int i;
      for (i = 0; i < strlen(text2); i++)
	    putchar(text2[i]);
      OSSemPost(printSem); // Release semaphore after printing is done
      OSTimeDlyHMSM(0, 0, 0, 4);
    }
}

void statisticTask(void* pdata)
{
  while(1)
    {
      printStackSize("Task1", TASK1_PRIORITY);
      printStackSize("Task2", TASK2_PRIORITY);
      printStackSize("StatisticTask", TASK_STAT_PRIORITY);
    }
}

int main(void)
{
  printSem = OSSemCreate(1); // Initialize Semaphore in main
  printf("Lab 3 - Two Tasks Improved\n");

  OSTaskCreateExt
    ( task1,
      NULL,
      &task1_stk[TASK_STACKSIZE-1],
      TASK1_PRIORITY,
      TASK1_PRIORITY,
      &task1_stk[0],
      TASK_STACKSIZE,
      NULL,
      OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
      );

  OSTaskCreateExt
    ( task2,
      NULL,
      &task2_stk[TASK_STACKSIZE-1],
      TASK2_PRIORITY,
      TASK2_PRIORITY,
      &task2_stk[0],
      TASK_STACKSIZE,
      NULL,
      OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
      );  

  if (DEBUG == 1)
    {
      OSTaskCreateExt
	( statisticTask,
	  NULL,
	  &stat_stk[TASK_STACKSIZE-1],
	  TASK_STAT_PRIORITY,
	  TASK_STAT_PRIORITY,
	  &stat_stk[0],
	  TASK_STACKSIZE,
	  NULL,
	  OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR
	  );
    }  

  OSStart();
  return 0;
}
