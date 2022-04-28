#include <stdint.h>

#define LED_GREEN 		12
#define LED_ORANGE 		13
#define LED_RED			14
#define LED_BLUE		15

#define DELAY_MS_COUNT		1250


void delay(uint32_t);
void led_init();
void led_on(uint8_t);
void led_off(uint8_t);

