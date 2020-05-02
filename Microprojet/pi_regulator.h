#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

float get_distance_cm(void);
uint16_t get_line_position(void);
uint8_t get_movement(void);
void process_image_start(void);
void pi_regulator_start(void);

#endif /* PROCESS_IMAGE_H */
