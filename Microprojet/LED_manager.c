#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <leds.h>
#include <LED_manager.h>
#include <move_car.h>

static THD_WORKING_AREA(waLedManager, 256);
static THD_FUNCTION(LedManager, arg) {
	
	chRegSetThreadName(__FUNCTION__);
    (void)arg;
    
    uint8_t movement;
    uint8_t turning;
    systime_t time;
    
    while(1)
    {
    	time = chVTGetSystemTime();
    	movement = get_movement();
        turning = get_turning();


        if(movement == MOV_STOP)
        {
        	set_body_led(0);
        	set_front_led(1);
        	//set_led(LED5, 1); //set back led (red)
        }

        else if(movement == MOV_CONTINUE || movement == MOV_START)
        {
        	set_front_led(0);
			set_body_led(1);
        }

        if(turning == LEFT)
        {
			set_led(LED7, 2);//toggle
			set_led(LED3,0);
        }

        else if(turning==RIGHT)
		{
			set_led(LED3,2);//toggle
			set_led(LED7,0);
		}
        else if(!turning)
        {
			set_led(LED3, 0);
			set_led(LED7,0);
        }
        chThdSleepUntilWindowed(time, time + MS2ST(250));
    }

}

void led_manager_start(void){
	chThdCreateStatic(waLedManager, sizeof(waLedManager), NORMALPRIO, LedManager, NULL);
}
