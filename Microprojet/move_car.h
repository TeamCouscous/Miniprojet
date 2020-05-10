#ifndef MOVE_CAR_H
#define MOVE_CAR_H


/**
* @brief   Sets speed from speed_max/MAX_COUNTER until speed_max values depending on the counter value
*
* @param 	speed_max		Maximum speed value used by the motors
* 			counter			Counter (=0 if the robot doesn't move) and is incremented while the black line is detected
*
* @return				New speed value
*/
int16_t set_speed(int16_t speed_max, uint8_t counter);

/**
* @brief   Changes the speed based on the selector or the back proximity sensors or the tilt of the floor.
* 			If the back proximity sensors detect an object -> speed_max=MAX_SPEED
* 			If the robot is tilted upwards -> speed_max=MAX_SPEED/4
* 			If the robot is tilted downwards -> speed_max=MAX_SPEEd
* 			If the selector is turned right, the speed increases by 200.
* 			If the selector is turned left, the speed decreases by 200.
* 			The speed possible values are -200, 0, 200, 400, 600, 800.
*
* @param 	speed_max		Maximum speed value used by the motors
*			select_state	Selector state ranges from 0 to 15
*			incline			Incline state: UP, DOWN or PLANE
*			accelerate		True if an object is detected by the back sensors
*
* @return	Speed_max value
*/
int16_t change_speed(int16_t speed_max, int16_t old_speed, uint8_t incline, bool accelerate);


/**
 *  @brief   Returns the turning state based on the right and left speed
 *
 *  @return		turning value: LEFT, RIGHT or NO
*/
uint8_t get_turning(void);

/**
 * @brief   Starts the move car thread
 */
void move_car_start(void);

#endif /* MOVE_CAR_H */
