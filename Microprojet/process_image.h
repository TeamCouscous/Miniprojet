#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

bool green_square(uint16_t i, uint8_t *red, uint8_t *green, uint8_t *blue);
bool red_square(uint16_t i, uint8_t *red, uint8_t *green, uint8_t *blue);
bool green_light(uint8_t *red, uint8_t *green, uint8_t *blue, bool previous_state);
uint16_t extract_line_width(uint8_t *buffer);
float get_distance_cm(void);
uint16_t get_line_position(void);
void process_image_start(void);

#endif /* PROCESS_IMAGE_H */
