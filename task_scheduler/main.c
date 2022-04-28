#include <stdint.h>
#include "led.h"

/*
 *	Task 1 will toggle Green LED at rate of 1s
 *	Task 2 will toggle Orange LED at rate of 500ms
 *	Task 3 will toggle Red LED at rate of 250ms
 *	Task 4 will toggle Blue LED at rate of 125ms
 *	The delays implemented are software delays using for loop
 */

#define HSI_CLOCK		16000000U
#define DUMMY_XPSR		(0x01000000)
#define NUM_TASKS		5
#define STACK_START		0x20000000
#define STACK_SIZE		(128 * 1024)
#define STACK_END		((STACK_START) + (STACK_SIZE))
#define TASK_STACK_SIZE		(1024)
#define IDLE_STACK_START	STACK_END
#define T1_STACK_START		(STACK_END - TASK_STACK_SIZE)
#define T2_STACK_START		(STACK_END - 2*TASK_STACK_SIZE)
#define T3_STACK_START		(STACK_END - 3*TASK_STACK_SIZE)
#define T4_STACK_START		(STACK_END - 4*TASK_STACK_SIZE)
#define SCHEDULER_STACK_START	(STACK_END - 5*TASK_STACK_SIZE)


void idle_task();
void task1_handler();
void task2_handler();
void task3_handler();
void task4_handler();
typedef void (*fptr)(void);
uint32_t get_psp();
void set_psp(uint32_t);
void update_next_task();
void schedule();
void unblock_tasks();

typedef enum {
	TASK_READY=0,TASK_BLOCKED
}state_t;

typedef struct {
	uint32_t psp;
	fptr handler;
	state_t state;
	uint32_t block_count;
}task_t;



uint32_t task_stacks[NUM_TASKS] = {IDLE_STACK_START,T1_STACK_START,T2_STACK_START,T3_STACK_START,T4_STACK_START};
fptr handlers[NUM_TASKS] = {idle_task,task1_handler,task2_handler,task3_handler,task4_handler};
task_t tasks[NUM_TASKS];
uint32_t current_task = 1;
uint32_t ticks = 0;

void task_delay(uint32_t count) {
	if(current_task) {
		tasks[current_task].block_count = ticks + count;
		tasks[current_task].state = TASK_BLOCKED;
		schedule();
	}
}

__attribute__((naked)) void switch_sp_to_psp() {
	asm volatile("PUSH {LR}");
	asm volatile("BL get_psp");
	asm volatile("MSR PSP,R0");
	asm volatile("MOV R0,#0x02");
	asm volatile("POP {LR}");
	asm volatile("MSR CONTROL,R0");
	asm volatile("BX LR");	
}

void systick_init() {
	uint32_t count = HSI_CLOCK/1000 - 1;//To get 1ms counts
	uint32_t *pSYSTICK_reload = (uint32_t *)0xE000E014;
	uint32_t *pSYSTICK_cntrl_status = (uint32_t *)0xE000E010;

	*pSYSTICK_reload &= ~(0x00FFFFFF);
	*pSYSTICK_reload |= count;

	*pSYSTICK_cntrl_status |= (1 << 1);
	*pSYSTICK_cntrl_status |= (1 << 2);

	*pSYSTICK_cntrl_status |= (1 << 0);
}

__attribute__((naked)) void PendSV_handler(void) {
	//Saving context of current task
	asm volatile("MRS R0, PSP");
	//Save R4-R11 into stack of current_task pointed by R0
	asm volatile("STMDB R0!, {R4-R11}");
	asm volatile("PUSH {LR}");
	asm volatile("BL set_psp");
	asm volatile("BL update_next_task");

	//Loading context of next task
	asm volatile("BL get_psp");
	//Load contents of R4-R11 for new task into the registers from the task's PSP pointed to by R0
	asm volatile("LDMIA R0!,{R4-R11}");
	asm volatile("MSR PSP,R0");

	asm volatile("POP {LR}");
	asm volatile("BX LR");
}

void SysTick_handler(void) {
	ticks++;
	unblock_tasks();
	schedule();
}

void init_tasks() {
	for(int i = 0; i < NUM_TASKS; i++) {
		tasks[i].handler = handlers[i];
		tasks[i].state = TASK_READY;
		tasks[i].block_count = 0;
		//Populating dummy values in the task's stack frame
		uint32_t *pPSP = (uint32_t *)task_stacks[i];

		pPSP--;
		*pPSP = DUMMY_XPSR;

		pPSP--;
		*pPSP = (uint32_t)handlers[i];

		pPSP--;
		*pPSP = 0xFFFFFFFD;

		for(int j = 0; j < 13; j++) {
			pPSP--;
			*pPSP = 0;
		}
		tasks[i].psp = (uint32_t)pPSP;

	}
}

//Initializing scheduler stack
__attribute__((naked)) void scheduler_init(uint32_t scheduler_stack_start) {
	asm volatile("MSR MSP,R0");
	asm volatile("BX LR");
}

uint32_t get_psp() {
	return tasks[current_task].psp;
}

void set_psp(uint32_t psp) {
	tasks[current_task].psp = psp;
}

void update_next_task() {
	uint32_t next_task = 0;
	for(int i = 0; i < NUM_TASKS; i++) {
		current_task++;
		if(current_task % NUM_TASKS == 0)
			current_task = 1;
		if(tasks[current_task].state == TASK_READY) {
			next_task = current_task;
			break;
		}
	}
	current_task = next_task;
}

void schedule() {
	//Pend the PendSV exception
	uint32_t *pICSR = (uint32_t *)0xE000ED04;
	*pICSR |= (1 << 28);
}

void unblock_tasks() {
	for(int i = 0; i < NUM_TASKS; i++) {
		if(tasks[i].block_count <= ticks) {
			tasks[i].state = TASK_READY;
		}
	}
}


void idle_task() {
	while(1);
}

void task1_handler() {
	while(1) {
		led_on(LED_GREEN);
		task_delay(1000);
		led_off(LED_GREEN);
		task_delay(1000);
	}
}

void task2_handler() {
	while(1) {
		led_on(LED_ORANGE);
		task_delay(500);
		led_off(LED_ORANGE);
		task_delay(500);
	}
}

void task3_handler() {
	while(1) {
		led_on(LED_RED);
		task_delay(250);
		led_off(LED_RED);
		task_delay(250);
	}
}

void task4_handler() {
	while(1) {
		led_on(LED_BLUE);
		task_delay(125);
		led_off(LED_BLUE);
		task_delay(125);
	}
}


int main() {
	scheduler_init(SCHEDULER_STACK_START);
	systick_init();
	led_init();
	init_tasks();
	switch_sp_to_psp();	
	task1_handler();
	while(1);
}
