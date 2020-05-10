#include "ch.h"
#include "hal.h"
#include <usbcfg.h>
#include <chprintf.h>

#include <selector.h>
#include <sensors/imu.h>
#include <motors.h>
#include <leds.h>
#include <msgbus/messagebus.h>
#include <i2c_bus.h>

#include <main.h>
#include <process_image.h>
#include <move_car.h>
#include <sensors.h>


static int16_t speed_right = 0;
static int16_t speed_left = 0;
static uint8_t select_state;

/***************************INTERNAL FUNCTIONS************************************/

/**
* @brief   Thread which controls the motors
*/

static THD_WORKING_AREA(waMoveCar, 4096);
static THD_FUNCTION(MoveCar, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed_m =0;
    int16_t speed_correction =0;
    int16_t speed=0;
    int16_t old_speed=0;

    select_state=get_selector();
    uint8_t count_speed=0;;
    uint8_t count_no_line=0;
    uint8_t incline=0;
    bool accelerate=0;


    while(1){
        time = chVTGetSystemTime();
        incline=get_inclined();
        accelerate=get_accelerate();
        speed_m = change_speed(speed_m,old_speed,incline,accelerate);

        //If the robot isn't moving on an inclined plane
        //and the back sensors do not detect an object
        if(accelerate==0 && incline ==PLANE){
        	old_speed=speed_m;
        }

        if(get_movement()==MOV_STOP || speed_m!=0){
       		//If proximity sensors on the left detect an object
       		if(get_turn_around() == LEFT)
       		{
       			speed_right = MAX_SPEED/4;
       			speed_left = - MAX_SPEED/4;
       		}
        	//If proximity sensors on the right detect an object
       		else if(get_turn_around() == RIGHT)
       		{
       			speed_right = -MAX_SPEED/4;
       			speed_left = MAX_SPEED/4;
       		}
        	//If the line is detected and no obstacle is detected
       		else if(get_line_position()<IMAGE_BUFFER_SIZE && get_line_position()>0 && speed_m>0 ){

       			count_no_line=0; // a line is detected

       			//Speed correction modifies the speed depending on the line position
       			speed_correction = (get_line_position()- (IMAGE_BUFFER_SIZE/2));
       			//if the line is nearly in front of the camera, don't rotate
        		if(abs(speed_correction) < ROTATION_THRESHOLD){
        			speed_correction = 0;
       			}
       			if(count_speed<MAX_COUNTER)
       				count_speed++;
       			//Starts the speed from speed_m/MAX_COUNTER to speed_m
       			speed=set_speed(speed_m, count_speed);

       			speed_right=speed - ROTATION_COEFF * speed_correction;
       			speed_left=speed + ROTATION_COEFF * speed_correction;

       		}else if(speed_m<0)
           	{
           		speed_right=speed_left=speed_m;
           	}
       		else{
       			if(count_no_line==50){ //turn left until finds line when no line has been detected 50 times
        			count_speed=0;
       				speed_right= -speed_m;
       				speed_left= speed_m;
       			}else{
       				speed_right=speed_left=0;
       				count_no_line++; //no line is detected
       			}
       		}
       	}else{
       		speed_right=speed_left=0;
       	}

        right_motor_set_speed(speed_right);
        left_motor_set_speed(speed_left);

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/



//sets speed from =speed_max/MAX_COUNTER to speed_max) steps/s depending on the counter value and stays at MAX_SPEED
int16_t set_speed(int16_t speed_max, uint8_t counter){
	//if counter equals MAX_COUNTER or exceeds this value
	if(counter > MAX_COUNTER){
		return speed_max;
	}else{
		return (counter)*(speed_max/MAX_COUNTER);
	}
}

//speed_m = {-200; 0; 200; 400; 600; 800}
int16_t change_speed(int16_t speed_max, int16_t old_speed, uint8_t inclined, bool accel)
{
	uint8_t old_select = select_state;
	select_state = get_selector();
	int8_t dif = select_state - old_select;
	if(speed_max > 0 ){ //If the robot is moving backwards or not at all, inclining or touching the robot from the back doesn't change its speed
		if(accel || inclined==DOWN) //If going down or the back proximity sensor is touched -> speed_m=800
			return MAX_SPEED;
		else if(inclined==UP)
			return MAX_SPEED/8; //if going up -> speed_m=200
		else
			speed_max=old_speed;
	}else if(speed_max<0){
		if(accel) //if moving backwards and an object is detected from the back -> moves forward
			return MAX_SPEED/4;
	}
	//if the selector isn't turned -> no change
	if(!dif)
		return speed_max;

	//if the selector is turned right
	else if((dif>0 && dif<9) || dif<-8)
		speed_max+=200;

	//if the selector is turned left
	else if((dif<0 && dif>-9) || dif>8)
		speed_max-=200;

	// Maximum speed doesn't exceed MAX_SPEED
	if(speed_max > MAX_SPEED)
		return MAX_SPEED;

	//Minimum speed is BACKWARD SPEED
	else if(speed_max < BACKWARD_SPEED)
		return BACKWARD_SPEED;

	//Maximum speed is in range
	else
		return speed_max;
}



uint8_t get_turning(void){
	if(speed_right<0 && speed_left<0) //If the robot is moving backwards
		return BACK;
	else if(speed_right==0 && speed_left==0) //If the robot isn't moving
		return STOP;
	else if(abs(speed_right-speed_left)<SPEED_THRESHOLD) //If the robot is moving straight
		return STRAIGHT;
	else if(speed_right>speed_left) //If the robot is turning left
		return LEFT;
	else	//If the robot is moving right
		return RIGHT;
}

void move_car_start(void){
	chThdCreateStatic(waMoveCar, sizeof(waMoveCar), NORMALPRIO, MoveCar, NULL);
}

/**************************END PUBLIC FUNCTIONS***********************************/
