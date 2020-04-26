#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>
#include <leds.h>
#include <process_image.h>


static float distance_cm = 0;
static uint8_t movement;
static uint16_t line_position = IMAGE_BUFFER_SIZE/2;	//middle

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);
static BSEMAPHORE_DECL(img_center_ready_sem, TRUE);
static BSEMAPHORE_DECL(img_bottom_ready_sem, TRUE);
static BSEMAPHORE_DECL(img_capture, TRUE);
/*
 *  Returns the line's width extracted from the image buffer given
 *  Returns 0 if line not found
 */
/*
bool green_square(uint16_t i, uint8_t *red, uint8_t *green, uint8_t *blue)
{
	uint8_t j=i;
	uint8_t pxl_count = 0;

	while(pxl_count <9)
	{
		if(green[j]> 4*(blue[j]+red[j]) && green[j] > 20)
		{
			pxl_count ++;

			if(pxl_count == 9)
				return true;

			else if(pxl_count == 3 || pxl_count == 6)
				j = j+18;

			else
				j=j+1;
		}

		else
			return false;
	}
	return false;
}


bool red_square(uint16_t i, uint8_t *red, uint8_t *green, uint8_t *blue)
{
	uint8_t j=i;
	uint8_t pxl_count = 0;

	while(pxl_count < 9)
	{
		if(red[j] > 2*(green[j]/2+blue[j]) && red[j]>10)
		{
			pxl_count ++;

			if(pxl_count == 9)
				return true;

			else if(pxl_count == 3 || pxl_count == 6)
				j = j+18;

			else
				j=j+1;
		}

		else
			return false;
	}
	return false;
}


bool green_light(uint8_t *red, uint8_t *green, uint8_t *blue, bool previous_state)
{
	//search for a green/red square of at least 3x3 pixels ?
	for(uint16_t i=0; i<400; i++)
	{
		//light detection, to adjust
		if(red_square(i, red, green, blue))
			return false;

		else if(green_square(i, red, green, blue))
			return true;

		else
			return previous_state;
	}
}*/

//checks if a light is turned on in the right side of the captured line
uint8_t get_light(uint8_t *red, uint8_t *green, uint8_t *blue, uint8_t mov){
	uint8_t red_pxl=0, blue_val, green_val; // green_pxl=0;

	//To look for light only in the second half of the line
	for(uint16_t i= IMAGE_BUFFER_SIZE/2; i< IMAGE_BUFFER_SIZE;i++){
		if(green[i] < 5 ){
			red_pxl+=1;
			if(red_pxl==50){
				return MOV_STOP;
			}
		}else
			red_pxl=0;
		/*while(green[i] > 4*(red[i]+blue[i]) && green[i]>10){
			green_pxl+=1;
			i++;
			if(green_pxl==50){
				if(mov==MOV_STOP){
					return MOV_START;
				}else
					return MOV_CONTINUE;
			}
		}*/
	}
	if(mov==MOV_STOP)
		return MOV_STOP;
	else
		return MOV_CONTINUE;
}

uint16_t extract_line_width(uint8_t *buffer){

	uint16_t i = 0, begin = 0, end = 0, width = 0;
	uint8_t stop = 0, wrong_line = 0, line_not_found = 0;
	uint32_t mean = 0;

	static uint16_t last_width = PXTOCM/GOAL_DISTANCE;

	//performs an average
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		mean += buffer[i];
	}
	mean /= IMAGE_BUFFER_SIZE;

	do{
		wrong_line = 0;
		//search for a begin
		while(stop == 0 && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
		{
			//the slope must at least be WIDTH_SLOPE wide and is compared
		    //to the mean of the image
		    if(buffer[i] > mean && buffer[i+WIDTH_SLOPE] < mean)
		    {
		        begin = i;
		        stop = 1;
		    }
		    i++;
		}
		//if a begin was found, search for an end
		if (i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) && begin)
		{
		    stop = 0;

		    while(stop == 0 && i < IMAGE_BUFFER_SIZE)
		    {
		        if(buffer[i] > mean && buffer[i-WIDTH_SLOPE] < mean)
		        {
		            end = i;
		            stop = 1;
		        }
		        i++;
		    }
		    //if an end was not found
		    if (i > IMAGE_BUFFER_SIZE || !end)
		    {
		        line_not_found = 1;
		    }
		}
		else//if no begin was found
		{
		    line_not_found = 1;
		}

		//if a line too small has been detected, continues the search
		if(!line_not_found && (end-begin) < MIN_LINE_WIDTH){
			i = end;
			begin = 0;
			end = 0;
			stop = 0;
			wrong_line = 1;
		}
	}while(wrong_line);

	if(line_not_found){
		begin = 0;
		end = 0;
		width = last_width;
	}else{
		last_width = width = (end - begin);
		line_position = (begin + end)/2; //gives the line position.
	}

	//sets a maximum width or returns the measured width
	if((PXTOCM/width) > MAX_DISTANCE){
		return PXTOCM/MAX_DISTANCE;
	}else{
		return width;
	}
}

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

	   chRegSetThreadName(__FUNCTION__);
	    (void)arg;

		//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 100 + 101 (minimum 2 lines because reasons)
		po8030_advanced_config(FORMAT_RGB565, 0,10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
		dcmi_enable_double_buffering();
		dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
		dcmi_prepare();

	    while(1){

	        //starts a capture
			dcmi_capture_start();
			//waits for the capture to be done
			wait_image_ready();

			//signals an image has been captured
			chBSemSignal(&image_ready_sem);
	    }
	}

static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_ctr_buff_ptr;
	//uint8_t *img_bot_buff_ptr;
	//uint8_t clr_intensity[IMAGE_BUFFER_SIZE] = {0};
	uint8_t red_ctr[IMAGE_BUFFER_SIZE] = {0};
	uint8_t green_ctr[IMAGE_BUFFER_SIZE] = {0};
	uint8_t blue_ctr[IMAGE_BUFFER_SIZE] = {0};

	uint16_t lineWidth = 0;

	movement = MOV_START;
	bool send_to_computer = true;

    while(1){
    	//waits until center has been captured
    	chBSemWait(&image_ready_sem);
    	//gets the pointer to the array filled with the last image in RGB565
    	img_ctr_buff_ptr = dcmi_get_last_image_ptr();

		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			//red : extracts first 5bits of the first byte, put them to the right.
        	red_ctr[i/2] = ((uint8_t)img_ctr_buff_ptr[i]&0xF8)>>3;
        	//green : extracts last 3 bits of the first byte, put them to the left then add the first 3 bits of the second byte shifted to the right.
        	green_ctr[i/2] = (((uint8_t)img_ctr_buff_ptr[i]&0x07)<<3) + (((uint8_t)img_ctr_buff_ptr[i+1]&0xE0)>>5);
        	//blue : extracts last 5 bits of the second byte.
        	blue_ctr[i/2] = (uint8_t)img_ctr_buff_ptr[i+1]&0x1F;
		}


		//search for a line in the image and gets its width in pixels
		lineWidth = extract_line_width(red_ctr);

		//search for light
		movement=get_light(red_ctr,green_ctr,blue_ctr,movement);

		if(movement==MOV_START)
			set_body_led(1);
		else if(movement==MOV_STOP)
			set_front_led(1);
		else{
			set_front_led(0);
			set_body_led(0);
		}

		//converts the width into a distance between the robot and the camera
		if(lineWidth){
			distance_cm = PXTOCM/lineWidth;
		}

		if(send_to_computer){
			//sends to the computer the image
			SendUint8ToComputer(red_ctr, IMAGE_BUFFER_SIZE);
		}
		//invert the bool
		send_to_computer = !send_to_computer;



    }
}

float get_distance_cm(void){
	return distance_cm;
}

uint16_t get_line_position(void){
	return line_position;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
