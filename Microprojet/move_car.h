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
* @brief   Changes the speed based on the selector.
* 			If the selector is turned right, the speed increases by 200.
* 			If the selector is turned left, the speed decreases by 200.
* 			The speed possible values are -200, 0, 200, 400, 600, 800.
*
* @param 	speed_max		Maximum speed value used by the motors
*			select_state	Selector state ranges from 0 to 15
*
* @return	Speed_max value
*/
int16_t change_speed(int16_t speed_max, uint8_t select_state);

void set_speed_control(void);


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
