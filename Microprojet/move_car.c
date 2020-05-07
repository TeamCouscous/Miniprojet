#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>

#include <selector.h>
#include <main.h>
#include <motors.h>
#include <process_image.h>
#include <proximity_sensor.h>
#include <leds.h>
#include <move_car.h>
#include <sensors/imu.h>
#include <msgbus/messagebus.h>
#include <i2c_bus.h>


static THD_WORKING_AREA(waMoveCar, 4096);
static THD_FUNCTION(MoveCar, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed;
    int16_t speed_m =0;
    int16_t speed_correction = 0;
    uint8_t count_no_line=0;

    uint8_t select_state=0;
    uint8_t count_speed=0;;

    float gravity_compensation;
    calibrate_acc();

    while(1){
        time = chVTGetSystemTime();


        gravity_compensation = get_acceleration(Y_AXIS)*G_COEFF;

        speed_m = change_speed(speed_m,select_state);

        	if(get_movement()!=MOV_STOP && !get_proximity_on()){ // { //&& get_movement()!=MOV_STOP
        		if(get_line_position()<IMAGE_BUFFER_SIZE && get_line_position()>0){	//&& get_movement()!=MOV_STOP
        				//speed_m = change_speed(speed_m,select_state);

        				speed_correction = (get_line_position()- (IMAGE_BUFFER_SIZE/2));
        				//if the line is nearly in front of the camera, don't rotate
        				if(abs(speed_correction) < ROTATION_THRESHOLD){
        					speed_correction = 0;
        				}
        				speed=set_speed(speed_m, count_speed);
        				right_motor_set_speed(speed_m - ROTATION_COEFF * speed_correction);
        				left_motor_set_speed(speed_m + ROTATION_COEFF * speed_correction);
        				count_no_line=0;
        		}else{
        			right_motor_set_speed(0);
        			left_motor_set_speed(0);
        			count_no_line++;
        			if(count_no_line>50){ //turn left until finds line
        				count_speed=0;
        				right_motor_set_speed(-200);
        				left_motor_set_speed(200);
        			}
        		}
        	}else{
        			right_motor_set_speed(0);
        		    left_motor_set_speed(0);
        	}
        //}
        /*
        if(gravity_compensation<400 && gravity_compensation>-400){
        	  right_motor_set_speed(speed+gravity_compensation);
        	  left_motor_set_speed(speed+gravity_compensation);
        }*/
        //chprintf((BaseSequentialStream *)&SD3, "gravity = %f\n",gravity_compensation);
        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

//sets speed from 4 (=MAX_SPEED/MAX_COUNTER) to 400 (=MAX_SPEED) steps/s depending on the counter value and stays at MAX_SPEED
int16_t set_speed(int16_t speed_max, uint8_t counter){
	//if counter equals MAX_COUNTER or exceeds this value
	if(counter + 20 == MAX_COUNTER){
		return speed_max;
	}else{
		counter++;
		return (counter+20)*(10*speed_max/MAX_COUNTER);
	}
}

//speed_m = {-200; 0; 200; 400; 600; 800}
int16_t change_speed(int16_t speed_max, uint8_t select_state)
{
	uint8_t old_select = select_state;
	select_state = get_selector();
	int8_t dif = select_state - old_select;
	if(!dif)
		return speed_max;

	else if((dif>0 && dif<9) || dif<-8)
		speed_max+=200;

	else if((dif<0 && dif>-9) || dif>8)
		speed_max-=200;

	if(speed_max > MAX_SPEED)
		return MAX_SPEED;
	else if(speed_max < BACKWARD_SPEED)
		return BACKWARD_SPEED;
	else
		return speed_max;
}

uint8_t get_turning(void){
	/*if(linePosition-IMAGE_BUFFER_SIZE/2 > BLINKING_THRESHOLD)
		return RIGHT;
	else if(IMAGE_BUFFER_SIZE/2-linePosition > BLINKING_THRESHOLD)
		return LEFT;
	else
		return NO;*/
	return NO;
}

void move_car_start(void){
	chThdCreateStatic(waMoveCar, sizeof(waMoveCar), NORMALPRIO, MoveCar, NULL);
}
