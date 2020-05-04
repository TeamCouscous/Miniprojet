#ifndef PROXIMITY_SENSOR_H_
#define PROXIMITY_SENSOR_H_

//start the Proximity sensor thread
void proximity_sensor_start(void);

//get bool proximity_on that detects if an object is in proximity of the robot
bool get_proximity_on(void);


#endif /* PROXIMITY_SENSOR_H_ */
