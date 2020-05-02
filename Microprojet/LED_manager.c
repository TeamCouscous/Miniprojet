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
    
    while(1)
    {
        movement = get_movement();
        if(movement == MOV_STOP)
        {
        	set_body_led(1);
        	set_front_led(0);
        }

        else if(movement == MOV_CONTINUE || movement == MOV_START)
        {
        	set_front_led(1);
			set_body_led(0);
        }
    }

}

void led_manager_start(void){
	chThdCreateStatic(waLedManager, sizeof(waLedManager), NORMALPRIO, LedManager, NULL);
}
