#include "ucos_ii.h"
#include <stdio.h>
#include "includes.h"
#include <string.h>
#include "altera_avalon_performance_counter.h"
#include "system.h"
#include "sys/alt_irq.h"

#define MEASURE_SEMAPHORE_POST_PEND 1
#define MEASURE_CONTEXT_SWITCH_0_TO_1 2
#define MEASURE_CONTEXT_SWITCH_1_TO_0 3
#define PERFORMANCE_COUNTER_0_BASE 0x9000

// rest of your code goes here

// Uncomment to limit the number of iterations
#define LIMIT_ITERATIONS 20

// Define mutex semaphore
OS_EVENT *task0StateSemaphore;
OS_EVENT *task1StateSemaphore;
OS_EVENT *measurementSemaphore;

/* Definition of Task Stacks */
/* Stack grows from HIGH to LOW memory */
#define TASK_STACKSIZE 2048
OS_STK task0_stk[TASK_STACKSIZE];
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK measurement_results_task_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define MEASUREMENT_RESULTS_TASK_PRIORITY 4
#define TASK0_PRIORITY 6
#define TASK1_PRIORITY 7

/* Prints a message and sleeps for given time interval */
void task0(void *pdata)
{
	INT8U err;
	int iterations = 0;
	while (iterations < 10)
	{
		OSSemPend(task1StateSemaphore, 0, &err);
		printf("Task 0 - State 0\n");

		// Start the timer after signaling but before the context switch from task 0 to task 1
		OSSemPost(task0StateSemaphore);
		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_0_TO_1);

		OSSemPend(task1StateSemaphore, 0, &err);
		// End the timer after the context switch from task 0 to task 1
		PERF_END(PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_0_TO_1);

		printf("Task 0 - State 1\n");

		// Start the timer after signaling but before the context switch from task 1 to task 0
		OSSemPost(task1StateSemaphore);
		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_1_TO_0);

		iterations++;
	}
	OSSemPost(measurementSemaphore);
}

void task1(void *pdata)
{
	INT8U err;
	int iterations = 0;

	while (iterations < 10)
	{
		// Start the timer after signaling but before the context switch from task 1 to task 0
		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_1_TO_0);
		OSSemPend(task0StateSemaphore, 0, &err);

		// End the timer after the context switch from task 1 to task 0
		PERF_END(PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_1_TO_0);

		printf("Task 1 - State 0\n");
		printf("Task 1 - State 1\n");

		OSSemPost(task1StateSemaphore);
		iterations++;
	}
	OSSemPost(measurementSemaphore);
}

void measurementResultsTask(void *pdata)
{

	INT8U err;
	// Wait for all tasks to be done before printing the results
	printf("Waiting for measurements..\n");
	OSSemPend(measurementSemaphore, 0, &err);
	printf("Measurements done, printing results:\n");

	PERF_STOP_MEASURING(PERFORMANCE_COUNTER_0_BASE);
	perf_print_formatted_report(
		(void *)PERFORMANCE_COUNTER_0_BASE, // Peripheral's HW base address
		ALT_CPU_FREQ,						// defined in "system.h"
		3,									// How many sections to print
		"Sem. Post/Pend",					// Display-names of sections
		"Task 0 to 1",						// Display-names of sections
		"Task 1 to 0"						// Display-names of sections
	);

	alt_u64 averageSemPostPendCycles = perf_get_section_time((void *)PERFORMANCE_COUNTER_0_BASE, MEASURE_SEMAPHORE_POST_PEND) / perf_get_num_starts((void *)PERFORMANCE_COUNTER_0_BASE, MEASURE_SEMAPHORE_POST_PEND);
	alt_u64 averageTask0to1ContextSwitchCycles = perf_get_section_time((void *)PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_0_TO_1) / perf_get_num_starts((void *)PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_0_TO_1);
	alt_u64 averageTask1to0ContextSwitchCycles = perf_get_section_time((void *)PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_1_TO_0) / perf_get_num_starts((void *)PERFORMANCE_COUNTER_0_BASE, MEASURE_CONTEXT_SWITCH_1_TO_0);

	alt_u64 averageContextSwitchCycles = (averageTask0to1ContextSwitchCycles + averageTask1to0ContextSwitchCycles) / 2;
	printf("\nContext switch average no. of CPU cycles: %i\n", (int)averageContextSwitchCycles);
	averageContextSwitchCycles -= averageSemPostPendCycles;
	printf("Context switch cpu cycle average minus semaphore post/pending function call cycles: %i\n", (int)averageContextSwitchCycles);
	float averageContextSwitchTime = (float)averageContextSwitchCycles / (float)alt_get_cpu_freq();
	printf("Average context switch time: %fs (%fus)\n", averageContextSwitchTime, averageContextSwitchTime * 1000000);
}

/* The main function creates two task and starts multi-tasking */
int main(void)
{
	printf("Lab 3 - Handshake\n");

	// Reset (initialize to zero) all section counters and the global
	// counter of the performance_counter peripheral.
	PERF_RESET(PERFORMANCE_COUNTER_0_BASE);
	PERF_START_MEASURING(PERFORMANCE_COUNTER_0_BASE);

	// initialize semaphores
	task0StateSemaphore = OSSemCreate(0);  // Initialize with state = 0
	task1StateSemaphore = OSSemCreate(1);  // Initialize with state = 1
	measurementSemaphore = OSSemCreate(0); // When updated, print the measurement results

	INT8U err;

	int x = 0;
	for (x = 0; x < 100; x++)
	{
		PERF_BEGIN(PERFORMANCE_COUNTER_0_BASE, MEASURE_SEMAPHORE_POST_PEND);
		OSSemPost(task0StateSemaphore);
		OSSemPend(task0StateSemaphore, 0, &err);
		PERF_END(PERFORMANCE_COUNTER_0_BASE, MEASURE_SEMAPHORE_POST_PEND);
	}

	// measure semaphore function calls

	OSTaskCreateExt(task0,							// Pointer to task code
					NULL,							// Pointer to argument passed to task
					&task0_stk[TASK_STACKSIZE - 1], // Pointer to top of task stack
					TASK0_PRIORITY,					// Desired Task priority
					TASK0_PRIORITY,					// Task ID
					&task0_stk[0],					// Pointer to bottom of task stack
					TASK_STACKSIZE,					// Stacksize
					NULL,							// Pointer to user supplied memory (not needed)
					OS_TASK_OPT_STK_CHK |			// Stack Checking enabled
						OS_TASK_OPT_STK_CLR			// Stack Cleared
	);

	OSTaskCreateExt(task1,							// Pointer to task code
					NULL,							// Pointer to argument passed to task
					&task1_stk[TASK_STACKSIZE - 1], // Pointer to top of task stack
					TASK1_PRIORITY,					// Desired Task priority
					TASK1_PRIORITY,					// Task ID
					&task1_stk[0],					// Pointer to bottom of task stack
					TASK_STACKSIZE,					// Stacksize
					NULL,							// Pointer to user supplied memory (not needed)
					OS_TASK_OPT_STK_CHK |			// Stack Checking enabled
						OS_TASK_OPT_STK_CLR			// Stack Cleared
	);

	OSTaskCreateExt(measurementResultsTask,							   // Pointer to task code
					NULL,											   // Pointer to argument passed to task
					&measurement_results_task_stk[TASK_STACKSIZE - 1], // Pointer to top of task stack
					MEASUREMENT_RESULTS_TASK_PRIORITY,				   // Desired Task priority
					MEASUREMENT_RESULTS_TASK_PRIORITY,				   // Task ID
					&measurement_results_task_stk[0],				   // Pointer to bottom of task stack
					TASK_STACKSIZE,									   // Stacksize
					NULL,											   // Pointer to user supplied memory (not needed)
					OS_TASK_OPT_STK_CHK |							   // Stack Checking enabled
						OS_TASK_OPT_STK_CLR							   // Stack Cleared
	);

	printf("Started...\n");

	OSStart();

	return 0;
}
