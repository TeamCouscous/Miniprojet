#ifndef PROXIMITY_SENSOR_H_
#define PROXIMITY_SENSOR_H_

/**
 * @brief   Starts the proximity sensor thread
 */
void proximity_sensor_start(void);
/**
 * @brief   Returns the boolean that shows if an object is in proximity of the robot
 *
 * @return	true if an object is detected, false if no object is detected
 */
bool get_proximity_on(void);


/**
 * @brief   Starts the imu sensor thread
 */
void imu_sensor_start(void);

int16_t get_g_compensation(void);

float get_acc_Y(void);

int8_t get_turn_around(void);

bool get_accelerate(void);


#endif /* PROXIMITY_SENSOR_H_ */
