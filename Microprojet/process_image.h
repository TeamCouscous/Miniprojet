#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

uint8_t get_light(uint8_t *red, uint8_t *green, uint8_t *blue);
uint16_t search_line_position(uint8_t *buffer);
uint16_t get_line_position(void);
uint8_t get_movement(void);
void process_image_start(void);

#endif /* PROCESS_IMAGE_H */
