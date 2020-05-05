#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>

#include <selector.h>
#include <main.h>
#include <motors.h>
#include <process_image.h>
#include <leds.h>
#include <move_car.h>
#include <sensors/imu.h>
#include <msgbus/messagebus.h>
#include <i2c_bus.h>


static THD_WORKING_AREA(waMoveCar, 256);
static THD_FUNCTION(MoveCar, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    int selector;

    systime_t time;

    int16_t speed;
    int16_t speed_correction = 0;
    uint8_t count_no_line=0;

    messagebus_topic_t *imu_topic = messagebus_find_topic_blocking(&bus, "/imu");
    imu_msg_t imu_values;

    while(1){
        time = chVTGetSystemTime();
        speed=400;
        selector = get_selector();

        messagebus_topic_wait(imu_topic, &imu_values, sizeof(imu_values));

        float gravity_compensation = imu_values.acceleration[Y_AXIS]*G_COEFF;

        //if(!get_proximity_on()){
        if(get_line_position()<IMAGE_BUFFER_SIZE && get_line_position()>0){	//&& get_movement()!=MOV_STOP
        			speed_correction = (get_line_position()- (IMAGE_BUFFER_SIZE/2));
        			//if the line is nearly in front of the camera, don't rotate
        			if(abs(speed_correction) < ROTATION_THRESHOLD){
		        	speed_correction = 0;
        			}
        			//speed=set_speed(count_speed);
        			right_motor_set_speed(speed - ROTATION_COEFF * speed_correction);
        			left_motor_set_speed(speed + ROTATION_COEFF * speed_correction);
        			count_no_line=0;
        }else{
        	right_motor_set_speed(0);
        	left_motor_set_speed(0);
        	count_no_line++;
        	if(count_no_line>50){ //turn left until finds line
        	    right_motor_set_speed(-200);
        	    left_motor_set_speed(200);
        	}
        }
        //}

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

//sets speed from 4 (=MAX_SPEED/MAX_COUNTER) to 400 (=MAX_SPEED) steps/s depending on the counter value and stays at MAX_SPEED
uint16_t set_speed(uint8_t counter){
	//if counter equals MAX_COUNTER or exceeds this value
	if(counter == MAX_COUNTER){
		return MAX_SPEED;
	}else{
		counter++;
		return counter*(MAX_SPEED/MAX_COUNTER);
	}
}


void move_car_start(void){
	chThdCreateStatic(waMoveCar, sizeof(waMoveCar), NORMALPRIO, MoveCar, NULL);
}
