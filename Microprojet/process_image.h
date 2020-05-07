#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

/**
 *  @brief   Returns the line position of a black line previously found
 *
 *  @return	line_position
*/
uint16_t get_line_position(void);
/**
 *  @brief   Returns the movement state value
 *
 *  @return	movement (MOV_START, MOV_STOP or MOV_CONTINUE)
*/
uint8_t get_movement(void);
/**
 * @brief   Starts the process image thread
 */
void process_image_start(void);

#endif /* PROCESS_IMAGE_H */
