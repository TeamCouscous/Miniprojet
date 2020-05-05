#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <sensors/proximity.h>

#include <main.h>
#include "proximity_sensor.h"
#include <process_image.h>

static bool proximity_on;	//If proximity_on=1, an object is close to the robot

static THD_WORKING_AREA(waProximitySensor, 2048);
static THD_FUNCTION(ProximitySensor, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while(1){
    	//calibrate_ir();
    	bool prox_on=0;
    	for(uint16_t  i=0; i<PROXIMITY_NB_CHANNELS; i++){
    		if(get_prox(i)> PROXIMITY_MAX){
    			prox_on=1;
    		}
    	}
    	proximity_on=prox_on;

    	//chprintf((BaseSequentialStream *)&SD3, "proximity on = %d\n", proximity_on);
    }
}


void proximity_sensor_start(void){
	chThdCreateStatic(waProximitySensor, sizeof(waProximitySensor), NORMALPRIO+1, ProximitySensor, NULL);
}

bool get_proximity_on(void){
	return proximity_on;
}
