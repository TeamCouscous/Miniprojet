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
#include <proximity_sensor.h>
#include <move_car.h>


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
    float g_comp = 0;

    select_state=get_selector();
    uint8_t count_speed=0;;
    uint8_t count_no_line=0;
    uint8_t count_turn_around = 0;



    while(1){
        time = chVTGetSystemTime();
        //int16_t g_comp = get_g_compensation();
        speed_m = change_speed(speed_m,old_speed);
        if(!get_accelerate())
        	old_speed=speed_m;

        	//if(get_movement()!=MOV_STOP){//&& !get_proximity_on()){

        		if(get_turn_around() == LEFT)
        		{
        			speed_right = 200;
        			speed_left = -200;
        		}

        		else if(get_turn_around() == RIGHT)
        		{
        			speed_left = 200;
        			speed_right = -200;
        		}

        		else if(get_line_position()<IMAGE_BUFFER_SIZE && get_line_position()>0){
        				//g_comp=get_acc_Y();
        				if(speed_m<0){
        					speed_correction = - (get_line_position()+ (IMAGE_BUFFER_SIZE/2));
        				}else{
        					speed_correction = (get_line_position()- (IMAGE_BUFFER_SIZE/2));
        				}
        				//if the line is nearly in front of the camera, don't rotate
        				if(abs(speed_correction) < ROTATION_THRESHOLD){
        					speed_correction = 0;
        				}
        				if(count_speed<MAX_COUNTER)
        					count_speed++;
        				speed=set_speed(speed_m, count_speed);
        				//speed=speed_m;
        				//g_comp = (int16_t)(get_acc_Y()*G_COEFF + speed_m/S_COEFF);
        				speed_right=speed - ROTATION_COEFF * speed_correction;
        				speed_left=speed + ROTATION_COEFF * speed_correction;
        				count_no_line=0;
        		}else{
        			if(count_no_line==50){ //turn left until finds line
        				count_speed=0;
        				speed_right= speed_m;
        				speed_left= speed_m;
        			}else{
        				speed_right=speed_left=0;
        				count_no_line++;
        			}
        		}
        	/*}else{
        		speed_right=speed_left=0;
        	}*/
        right_motor_set_speed(speed_right);
        left_motor_set_speed(speed_left);

        //chprintf((BaseSequentialStream *)&SD3, "get_acc_Y = %f\n", get_acc_Y());
        chprintf((BaseSequentialStream *)&SD3,"g_comp = %d\n", get_g_compensation());
        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

//sets speed from 4 (=MAX_SPEED/MAX_COUNTER) to 400 (=MAX_SPEED) steps/s depending on the counter value and stays at MAX_SPEED
int16_t set_speed(int16_t speed_max, uint8_t counter){
	//if counter equals MAX_COUNTER or exceeds this value
	if(counter > MAX_COUNTER){
		return speed_max;
	}else{
		return (counter)*(speed_max/MAX_COUNTER);
	}
}
/*
 *
 */
//speed_m = {-200; 0; 200; 400; 600; 800}
int16_t change_speed(int16_t speed_max, int16_t old_speed)
{
	bool accel = get_accelerate();
	uint8_t old_select = select_state;
	select_state = get_selector();
	int8_t dif = select_state - old_select;

	if(accel)
		return MAX_SPEED;
	else
		speed_max=old_speed;

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

void turn_around_left(void)
{

}

void turn_around_right(void)
{

}


/*************************END INTERNAL FUNCTIONS**********************************/


/****************************PUBLIC FUNCTIONS*************************************/



uint8_t get_turning(void){
	if(speed_right<0 && speed_left<0)
		return BACK;
	else if(speed_right==0 && speed_left==0)
		return STOP;
	else if(abs(speed_right-speed_left)<SPEED_THRESHOLD)
		return STRAIGHT;
	else if(speed_right>speed_left)
		return LEFT;
	else
		return RIGHT;
}

void move_car_start(void){
	chThdCreateStatic(waMoveCar, sizeof(waMoveCar), NORMALPRIO, MoveCar, NULL);
}

/**************************END PUBLIC FUNCTIONS***********************************/
