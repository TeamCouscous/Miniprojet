#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <process_image.h>
#include <leds.h>
#include <move_car.h>


static THD_WORKING_AREA(waMoveCar, 256);
static THD_FUNCTION(MoveCar, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0;

    while(1){
        time = chVTGetSystemTime();

        //computes the speed to give to the motors
        speed=400;

        /*
		//applies the correction for the rotation
		if(get_movement()!=MOV_STOP){
	   	   	   right_motor_set_speed(speed - ROTATION_COEFF * speed_correction);
	   	   	   left_motor_set_speed(speed + ROTATION_COEFF * speed_correction);
		}else{
			right_motor_set_speed(0);
			left_motor_set_speed(0);
		}*/

        if(get_line_position()<IMAGE_BUFFER_SIZE && get_line_position()>0){
        			speed_correction = (get_line_position()- (IMAGE_BUFFER_SIZE/2));
        			//if the line is nearly in front of the camera, don't rotate
        			if(abs(speed_correction) < ROTATION_THRESHOLD){
		        	speed_correction = 0;
        			}

        			right_motor_set_speed(speed - ROTATION_COEFF * speed_correction);
        			left_motor_set_speed(speed + ROTATION_COEFF * speed_correction);
        			k=0;
        }else{
        	if(k<50)
        	{
        		right_motor_set_speed(speed - ROTATION_COEFF * speed_correction);
        		left_motor_set_speed(speed + ROTATION_COEFF * speed_correction);
        		k++;
        	}else{
        		right_motor_set_speed(0);
        		left_motor_set_speed(0);
        	}
        }
        		/*
		for(uint16_t i=0; i<49;i++)
		{
			line_position[i]=line_position[i+1];
		}
		line_position[49] = get_line_position();*/
        	// chprintf((BaseSequentialStream *)&SD3, "linewidth = %d\n",lineWidth);







        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void move_car_start(void){
	chThdCreateStatic(waMoveCar, sizeof(waMoveCar), NORMALPRIO, MoveCar, NULL);
}
