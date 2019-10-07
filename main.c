#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include <chprintf.h>

#include "process_image.h"
#include "spi_comm.h"
#include "memory_protection.h"
#include "usbcfg.h"
#include <main.h>
#include "motors.h"
#include "camera/po8030.h"
#include "sensors/VL53L0X/VL53L0X.h"
#include "leds.h"
#include "audio/play_melody.h"
#include "audio/audio_thread.h"
#include "move.h"

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

// Robot's coordinates
static int pos_x = 0, pos_y = 0, pos_angle = 0;

int main(void)
{

    halInit();
    chSysInit();
    mpu_init();

    //starts the serial communication
    serial_start();
    //start the USB communication
    usb_start();
    //starts the camera
    dcmi_start();
	po8030_start();
	//inits the motors
	motors_init();
	//inits ESP32 for the leds
	spi_comm_start();
	//inits the leds
	clear_leds();
	//initialisations necessary for the melody
	dac_start();
	playMelodyStart();
	//inits the distance sensor
	VL53L0X_start();

	//stars the threads for the pi regulator and the processing of the image
	process_image_start();

	uint32_t distance = 0 ;
	uint16_t nb_line = 0;

	//main loop
    while(1) {

    	distance = VL53L0X_get_dist_mm();

    	//detects if the codes is at the right distance for an optimal bar code reading
    	if(distance <= MAX_DIST && distance >= MIN_DIST){
    		set_front_led(1);
    		nb_line = get_line_nb(); //number of lines of the bar code read by the camera
    		switch(nb_line){
    		case 6: // goes left a quarter turn, forward 10cm and blinks
    			left(QUARTER, left_motor_get_pos());
    			forward(FORWARD_UNIT, left_motor_get_pos());
    			pos_angle++;
    			update_pos();
    			light();
    			break;
    		case 10: // goes right a quarter turn, forward 10cm and blinks
    			right(QUARTER, left_motor_get_pos());
    			forward(FORWARD_UNIT, left_motor_get_pos());
    			pos_angle--;
    			update_pos();
    			light();
    			break;
    		case 14: // goes forward 10cm and blinks
    			forward(FORWARD_UNIT, left_motor_get_pos());
    			update_pos();
    			light();
    			break;
    		case 18: // goes back at the starting point and plays "Sandstorm"
    			go_back();
    			playMelody(SANDSTORMS, ML_SIMPLE_PLAY, NULL);
    			break;
    		default :
    			clear_leds();
    		}
    	} else{
    		set_front_led(0);
    	}
        chThdSleepMilliseconds(100);
    }
}


void light()
{
	for(int j=0; j<4; j++) {
		for(int i=0; i<4; i++) {
			toggle_rgb_led(i, j, 10);
			chThdSleepMilliseconds(50);
		}
		set_body_led(2);
	}
	set_front_led(2);
	clear_leds();
}

void update_pos(){

	//modified modulo for negative numbers
	int orientation=(pos_angle%NB_ORIENTATIONS+NB_ORIENTATIONS)%NB_ORIENTATIONS;

	//depending on the orientation, updates the position
	switch(orientation){
		case 0:
			pos_x++;
			break;
		case 1:
			pos_y++;
			break;
		case 2:
			pos_x--;
			break;
		case 3:
			pos_y--;
			break;
	}
}


void go_back(){

	int angle_back = 0; // rotation necessary to go back
	pos_angle = (pos_angle%NB_ORIENTATIONS+NB_ORIENTATIONS)%NB_ORIENTATIONS; //modified modulo for negative numbers

	//first the robot takes the orientation of the x axis
	angle_back = -pos_angle;

	//corrects the direction if the robot face the negative side of the axis
	if(pos_x >= 0){
		angle_back += HALF_TURN;
	}

	turn_back(angle_back); //turn to the correct orientation

	pos_angle += angle_back; //update the orientation once the rotation is done
	forward(FORWARD_UNIT*abs(pos_x), left_motor_get_pos()); //goes back to the initial position on the x axis


	//then the robot starts to go back to position y=0

	angle_back = -pos_angle-1; // orientation of the y axis

	//corrects the angle if the robot is on the other side of the axis (negative y)
	if(pos_y <= 0){
		angle_back += HALF_TURN;
	}

	turn_back(angle_back); //turn to the correct orientation

	pos_angle += angle_back; //update the orientation after the turn is done
	forward(FORWARD_UNIT*abs(pos_y), left_motor_get_pos()); // goes back to the origin point (0,0)

	//reset the position
	pos_x = 0;
	pos_y = 0;

	right(QUARTER*pos_angle, left_motor_get_pos()); //goes back to the original orientation
	pos_angle = 0; //reset orientation
}

void turn_back(int angle_back)
{
	if(angle_back >= 0){
		left(QUARTER*angle_back, left_motor_get_pos());
	} else{
		right(QUARTER*(-angle_back), left_motor_get_pos());
	}
}



#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
