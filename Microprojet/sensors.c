#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <sensors/proximity.h>
#include <sensors/imu.h>

#include <main.h>
#include <process_image.h>
#include <sensors.h>


static int8_t turn_around=0;
static bool accelerate = 0;
//static uint8_t inclined=0; ////Supprim� car non fonctionnel

/***************************INTERNAL FUNCTIONS************************************/

/**
* @brief   Thread which processes the sensor proximity measures
*/
static THD_WORKING_AREA(waProximitySensor, 1024);
static THD_FUNCTION(ProximitySensor, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
    systime_t time;

    uint16_t count_t=0;
    uint16_t count_a=0;

    while(1){
    	 time = chVTGetSystemTime();
    	 //When the front left proximity sensors detect an object
    	if(get_prox(6) > PROXIMITY_MAX || get_prox(7) > PROXIMITY_MAX)
    	{
    		turn_around = RIGHT;
    		count_t = 0;
    	}
    	//When the front right proximity sensors detect an object
    	else if(get_prox(0)> PROXIMITY_MAX || get_prox(1) > PROXIMITY_MAX)
    	{
    		turn_around = LEFT;
    		count_t = 0;
    	}
    	//turn around is reinitialized
    	else if(count_t == 300)
    	{
    		turn_around = 0;
    		count_t = 0;
    	}
    	//When the back proximity sensors detect an object
		if(get_prox(3) > PROXIMITY_MAX || get_prox(4) > PROXIMITY_MAX)
		{
			accelerate = 1;
			count_a = 0;
		}

		//accelerate is reinitialized
		else if(count_a > 200)
		{
			accelerate = 0;
			count_a = 0;
		}
		count_a ++;

		count_t ++;

    	chThdSleepUntilWindowed(time, time + MS2ST(10));

    }
}



/**
* @brief   Thread which processes the imu sensor measures: acceleration in order to know the direction of the incline
*/
/*
static THD_WORKING_AREA(waImuSensor, 1024);
static THD_FUNCTION(ImuSensor, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
    systime_t time;
    calibrate_acc();
    uint8_t count_up=0, count_down=0,count_plane=0;
    float angle=0;

    while(1){
    	 time = chVTGetSystemTime();
    	 float accel_X=get_acceleration(X_AXIS), accel_Y=get_acceleration(Y_AXIS);
    	 if(fabs(accel_X) > ANGLE_THRESHOLD || fabs(accel_Y) > ANGLE_THRESHOLD){
    		 //clock wise angle in rad with 0 being the back of the e-puck2 (Y axis of the IMU)
    		  angle = atan2(accel_X, accel_Y);

    		  //if the angle is greater than PI, then it has shifted on the -PI side of the quadrant
    		  //so we correct it
    		  if(angle > M_PI){
    			  angle = -2 * M_PI + angle;
    		  }

    		  //Depending on the angle, the direction of the incline is known
    	      if(angle >= 0 && angle < M_PI/4){
    	          count_up++;
    	          count_down=0;
    	          count_plane=0;
    	      }else if(angle >= 3*M_PI/4 && angle < M_PI){
    	    	  count_up=0;
    	    	  count_down++;
    	    	  count_plane=0;
    	      }else if(angle >= -M_PI && angle < -3*M_PI/4){
    	    	  count_up=0;
    	    	  count_down++;
    	    	  count_plane=0;
    	      }else if(angle >= -M_PI/4 && angle < 0){
    	    	  count_up++;
    	    	  count_down=0;
    	    	  count_plane=0;
    	      }
    	      else{
    	    	  count_up=0;
    	    	  count_down=0;
    	      }
    	      //If a certain direction is maintained 10 times
    	      if(count_up> 10){
    	    	  inclined = UP;
    	    	  count_up=50;
    	      }else if(count_down>10){
    	    	  inclined = DOWN;
    	    	  count_down=10;
    	      }else if(count_plane>10){
    	    	  inclined=PLANE;
    	    	  count_plane=10;
    	      }
    	 }



    	chThdSleepUntilWindowed(time, time + MS2ST(10));


    }
}*/ //Supprim� car non fonctionnel

/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/

void proximity_sensor_start(void){
	chThdCreateStatic(waProximitySensor, sizeof(waProximitySensor), NORMALPRIO+1, ProximitySensor, NULL);
}

/*
void imu_sensor_start(void){
	chThdCreateStatic(waImuSensor, sizeof(waImuSensor), NORMALPRIO+1, ImuSensor, NULL);
}*/ //Supprim� car non fonctionnel



int8_t get_turn_around(void){
	return turn_around;
}

bool get_accelerate(void){
	return accelerate;
}

/*
uint8_t get_inclined(void){
	return inclined;
}*/ //Supprim� car non fonctionnel

/**************************END PUBLIC FUNCTIONS***********************************/
