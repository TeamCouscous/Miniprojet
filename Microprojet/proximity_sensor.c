#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <sensors/proximity.h>
#include <sensors/imu.h>

#include <main.h>
#include "proximity_sensor.h"
#include <process_image.h>

static bool proximity_on;//If proximity_on=1, an object is close to the robot
static int16_t g_compensation;
static float acc;

/***************************INTERNAL FUNCTIONS************************************/

/**
* @brief   Thread which processes the sensor proximity measures
*/
static THD_WORKING_AREA(waProximitySensor, 2048);
static THD_FUNCTION(ProximitySensor, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
    systime_t time;


    while(1){
    	 time = chVTGetSystemTime();
    	bool prox_on=0;

    	for(uint8_t  i=0; i<PROXIMITY_NB_CHANNELS; i++){
    		if(get_prox(i)> PROXIMITY_MAX ){
    			prox_on=1;
    		}
    	}
    	proximity_on=prox_on;

    	chThdSleepUntilWindowed(time, time + MS2ST(10));

    	//chprintf((BaseSequentialStream *)&SD3, "proximity on = %d\n", proximity_on);
    }
}



static THD_WORKING_AREA(waImuSensor, 1024);
static THD_FUNCTION(ImuSensor, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
    systime_t time;
    calibrate_gyro();
    g_compensation = 0;

    while(1){
    	 time = chVTGetSystemTime();
    	 g_compensation = get_gyro(X_AXIS)-get_gyro_offset(X_AXIS);
    	 //acc=get_acceleration(Y_AXIS);
    	 //if(abs(gyro_X) > GYRO_MIN)

    	//g_compensation += gyro_X/G_COEFF;
    	 /*
    	 if(abs(g_compensation) < G_MIN)
    	 {
    		 g_compensation = 0;
    		 calibrate_gyro();
    	 }
	*/

    	chThdSleepUntilWindowed(time, time + MS2ST(10));


    }
}

/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/

void proximity_sensor_start(void){
	chThdCreateStatic(waProximitySensor, sizeof(waProximitySensor), NORMALPRIO+1, ProximitySensor, NULL);
}

void imu_sensor_start(void){
	chThdCreateStatic(waImuSensor, sizeof(waImuSensor), NORMALPRIO+1, ImuSensor, NULL);
}

bool get_proximity_on(void){
	return proximity_on;
}

int16_t get_g_compensation(void){
	return g_compensation;
}

float get_acc_Y(void){
	return acc;
}



/**************************END PUBLIC FUNCTIONS***********************************/
