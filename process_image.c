#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>

#include <main.h>
#include <camera/po8030.h>

#include <process_image.h>


static uint16_t nb_line=0;
//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

/*
 *  Returns the line's number extracted from the image buffer given
 *  Returns 0 if line not found
 */

uint16_t extract_line_nb(uint8_t *buffer){

	uint16_t i = 0, begin = 0, end = 0 ;
	uint8_t stop = 0, line_not_found = 0;
	uint32_t mean = 0;
	nb_line=0;

	//performs an average
	for(uint16_t i = 0 ; i < IMAGE_BUFFER_SIZE ; i++){
		mean += buffer[i];
	}
	mean /= IMAGE_BUFFER_SIZE;

	//search for lines until the end of the buffer is reached
	while(i<(IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
	{
		//look for the start of a line
		while(stop == 0 && i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE))
		{
			//the slope must be at least WIDTH_SLOPE wide and is compared
			//to the mean of the image
			if(buffer[i] > mean && buffer[i+WIDTH_SLOPE] < mean)
			{
				//begin is initialized at the point where the start of a line is found
				//so we can analyze the width of the line
				begin = i;
				stop = 1;
			}
			i++;
		}
		//if a begin has been found, search for an end
		if(i < (IMAGE_BUFFER_SIZE - WIDTH_SLOPE) && begin)
		{
			stop = 0;

			//we look for the end of the line as long as the end of the buffer is not reached
			while(stop == 0 && i < IMAGE_BUFFER_SIZE)
			{
				//condition for a line end
				if(buffer[i] < mean && buffer[i+WIDTH_SLOPE] > mean)
				{
					//end is initialized at the end of the line so we can
					//compare with begin to analyze the width of the line
					end = i;
					stop = 1;

					//if the line is too thin or too wide it is not counted
					//as it is not coherent with our expected width
					if((end-begin) > MIN_LINE_WIDTH && (end-begin) < MAX_LINE_WIDTH){
						nb_line++;
					}
				}
				i++;
			}
			//if no line was found
			if(i > IMAGE_BUFFER_SIZE && nb_line == 0)
			{
				//this is used at the end of the function to know if no line was found
				line_not_found = 1;
			}
		}

		else {
			line_not_found = 1;
		}

		stop = 0;
	}

	if(line_not_found){
		begin = 0;
		end = 0;
		nb_line = 0;
	}

	return nb_line ;
}

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the line 10 + 11 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, 10, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
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

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};


	while(1){
		//int i =0;
		//waits until an image has been captured
		chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565
		img_buff_ptr = dcmi_get_last_image_ptr();
		//Extracts only the red pixels
		for(uint16_t i = 0 ; i < (2 * IMAGE_BUFFER_SIZE) ; i+=2){
			//extracts first 5bits of the first byte
			//takes nothing from the second byte
			image[i/2] = (uint8_t)img_buff_ptr[i]&0xF8;
		}

		nb_line = extract_line_nb(image);
		chThdSleepMilliseconds(10);
	}
}

uint16_t get_line_nb(void){
	return nb_line;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}
