#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <leds.h>
#include <LED_manager.h>
#include <process_image.h>

static THD_WORKING_AREA(waLedManager, 256);
static THD_FUNCTION(LedManager, arg) {
	
	chRegSetThreadName(__FUNCTION__);
    (void)arg;
    
    uint8_t movement;
    
    movement = get_movement();
}

void led_manager_start(void){
	chThdCreateStatic(waLedManager, sizeof(waLedManager), NORMALPRIO, LedManager, NULL);
}