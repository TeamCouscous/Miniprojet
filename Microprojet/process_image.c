#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>
#include <leds.h>
#include <process_image.h>


//static float distance_cm = 0;
static uint8_t movement;
static uint16_t linePosition;
//static uint16_t line_position = IMAGE_BUFFER_SIZE/2;	//middle
//static uint8_t color;

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

//checks if a light is turned on in the right side of the captured line
uint8_t get_light(uint8_t *red, uint8_t *green, uint8_t *blue){
	uint8_t red_pxl=0, green_pxl=0;
	uint16_t i;

	//To look for light only in the second half of the line
	for(i= IMAGE_BUFFER_SIZE/2; i< IMAGE_BUFFER_SIZE;i++){
		if( red[i] > green[i]/2+blue[i] && red[i]> 10){
			red_pxl+=1;
			if(red_pxl==50){
				return MOV_STOP;
			}
		}else
			red_pxl=0;
		if(green[i] > 2*(red[i]+blue[i]) && green[i]>20){
			green_pxl++;
			if(green_pxl==50){
				if(movement==MOV_STOP){
						return MOV_START;
				}else
						return MOV_CONTINUE;
			}
		}else{
			green_pxl=0;
		}
	}
	if(movement==MOV_STOP)
		return MOV_STOP;
	else
		return MOV_CONTINUE;
}

uint16_t search_line_position(uint8_t *buffer){

	uint16_t i = 0, begin = 0, end = 0;
	uint8_t stop = 0, wrong_line = 0, line_not_found = 0;
	uint32_t mean = 0;

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
		    	end=IMAGE_BUFFER_SIZE;
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
		return -1;
	}
	else
		return (begin + end)/2; //gives the line position.
}

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

	   chRegSetThreadName(__FUNCTION__);
	    (void)arg;

		//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 479 + 480 (minimum 2 lines because reasons)
		po8030_advanced_config(FORMAT_RGB565, 0, 479, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
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

static THD_WORKING_AREA(waProcessImage, 4096);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_ctr_buff_ptr;
	//uint8_t *img_bot_buff_ptr;
	uint8_t clr_intensity[IMAGE_BUFFER_SIZE] = {0};
	uint8_t red[IMAGE_BUFFER_SIZE] = {0};
	uint8_t green[IMAGE_BUFFER_SIZE] = {0};
	uint8_t blue[IMAGE_BUFFER_SIZE] = {0};
	uint8_t clr_intensity[IMAGE_BUFFER_SIZE] ={0};


	//uint16_t lineWidth = 0;

	movement = MOV_STOP;
	//bool send_to_computer = true;

    while(1){
    	//waits until center has been captured
    	chBSemWait(&image_ready_sem);
    	//gets the pointer to the array filled with the last image in RGB565
    	img_ctr_buff_ptr = dcmi_get_last_image_ptr();


		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			//red : extracts first 5bits of the first byte, put them to the right.
        	red[i/2] = ((uint8_t)img_ctr_buff_ptr[i]&0xF8)>>3;
        	//blue : extracts last 5 bits of the second byte.
        	blue[i/2] = (uint8_t)((uint8_t)img_ctr_buff_ptr[i+1]&0x1F);
        	//green : extracts last 3 bits of the first byte, put them to the left then add the first 3 bits of the second byte shifted to the right.
        	green[i/2] = (uint8_t)(((uint8_t)img_ctr_buff_ptr[i]&0x07)<<3) + (((uint8_t)img_ctr_buff_ptr[i+1]&0xE0)>>5);
        	//clr_intensity
        	clr_intensity[i/2]=2*(red[i/2]+blue[i/2])+green[i/2];
		}

		//search for a line in the image and gets its width in pixels
		linePosition = search_line_position(clr_intensity);

		//search for light
		movement=get_light(red,green,blue);


		/*if(send_to_computer){
			//sends to the computer the image
			SendUint8ToComputer(clr_intensity, IMAGE_BUFFER_SIZE);
		}
		//invert the bool
		send_to_computer = !send_to_computer;*/




		/*chprintf((BaseSequentialStream *)&SD3, "green = %d\n",green_ctr[IMAGE_BUFFER_SIZE/2]);
		chprintf((BaseSequentialStream *)&SD3, "blue = %d\n",blue_ctr[IMAGE_BUFFER_SIZE/2]);
		chprintf((BaseSequentialStream *)&SD3, "red = %d\n",red_ctr[IMAGE_BUFFER_SIZE/2]);*/

		//chprintf((BaseSequentialStream *)&SD3, "movement = %d\n",movement);

    }
}


uint8_t get_movement(void){
	return movement;
}

uint16_t get_line_position(void){
	return linePosition;
}


void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
