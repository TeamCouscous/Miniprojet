#ifndef SENSORS_H_
#define SENSORS_H_

/**
 * @brief   Starts the proximity sensor thread
 */
void proximity_sensor_start(void);

/**
 * @brief   Starts the imu sensor thread
 */
void imu_sensor_start(void);

/**
 * @brief  returns  turn around value
 * 			0 if nothing is detected on the 4 front proximity sensors
 * 			LEFT if an object is detected by the right front proximity sensors
 * 			RIGHT if an object is detected by the left front proximity sensors
 */
int8_t get_turn_around(void);

/**
 * @brief   returns accelerate value
 * 			true if an object is detected by back proximity sensors
 * 			false if nothing is detected
 */
bool get_accelerate(void);

/**
 * @brief   returns the incline direction
 * 			UP if the robot is moving up
 * 			DOWN if the robot is moving down
 */
uint8_t get_inclined(void);



#endif /* SENSORS_H_ */
