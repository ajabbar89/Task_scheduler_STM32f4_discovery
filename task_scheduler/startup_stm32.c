#include <stdint.h>

#define SRAM_START		0x20000000U
#define SRAM_SIZE		(128U*1024U)
#define SRAM_END		((SRAM_START) + (SRAM_SIZE))
#define STACK_START		SRAM_END


extern uint32_t _start_data;
extern uint32_t _end_data;
extern uint32_t _start_bss;
extern uint32_t _end_bss;
extern uint32_t _end_text;
extern int main();

void Reset_handler();
void NMI_handler() __attribute__((weak,alias("Default_handler")));
void HardFault_handler() __attribute__((weak,alias("Default_handler")));
void MemManage_handler() __attribute__ ((weak,alias("Default_handler")));
void BusFault_handler() __attribute__((weak,alias("Default_handler")));
void UsageFault_handler() __attribute__((weak,alias("Default_handler")));
void SVC_handler() __attribute__((weak,alias("Default_handler")));
void DebugMon_handler() __attribute__((weak,alias("Default_handler")));
void PendSV_handler() __attribute__((weak,alias("Default_handler")));
void SysTick_handler() __attribute__((weak,alias("Default_handler")));

uint32_t vectors[] __attribute__((section (".isr_vectors")))= {
	STACK_START,
	(uint32_t)Reset_handler,
	(uint32_t)NMI_handler,
	(uint32_t)HardFault_handler,
	(uint32_t)MemManage_handler,
	(uint32_t)BusFault_handler,
	(uint32_t)UsageFault_handler,
	0,
	0,
	0,
	0,
	(uint32_t)SVC_handler,
	(uint32_t)DebugMon_handler,
	0,
	(uint32_t)PendSV_handler,
	(uint32_t)SysTick_handler
};


void Default_handler() {
	while(1);
}

void Reset_handler() {
	//Copy .data section to SRAM 
	uint32_t size = (uint32_t)&_end_data - (uint32_t)&_start_data;
	uint8_t *dest = (uint8_t *)&_start_data;
	uint8_t *src = (uint8_t *)&_end_text;
	for(int i = 0; i < size; i++) {
		*dest++ = *src++;
	}
	
	//init .bss section in SRAM
	size = (uint32_t)&_end_bss - (uint32_t)&_start_bss;
	dest = (uint8_t *)&_start_bss;
	for(int i = 0; i < size; i++)
	       *dest++ = 0;	

	//call main()
	main();
}



