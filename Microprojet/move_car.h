#ifndef MOVE_CAR_H
#define MOVE_CAR_H

enum{
	NO_CHANGE,
	SPEED_UP,
	SPEED_DOWN
};

//start the MOVE_CAR thread
void move_car_start(void);

int16_t set_speed(int16_t speed_max, uint8_t counter);
int16_t change_speed(int16_t speed_max, uint8_t select_state);

#endif /* MOVE_CAR_H */
