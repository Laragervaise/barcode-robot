#ifndef MAIN_H
#define MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "camera/dcmi_camera.h"
#include "msgbus/messagebus.h"
#include "parameter/parameter.h"


//constants for the differents parts of the project
#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5		//[px]
#define MIN_LINE_WIDTH			12		// to avoid lines that are the result of noise
#define MAX_LINE_WIDTH			300		// lines that are too big are not expected
#define MIN_DIST               	170		// [mm] the range of detection of the lines in mm
#define MAX_DIST              	230		//
#define QUARTER                	4.375   // quarter of a turn
#define FORWARD_UNIT      		10		// arbitrary distance [cm]
#define NB_ORIENTATIONS      	4		// for the go_back function
#define HALF_TURN      			2		// for the go_back function

/** Robot wide IPC bus. */
extern messagebus_t bus;

extern parameter_namespace_t parameter_root;

void SendUint8ToComputer(uint8_t* data, uint16_t size);


void light(void);// blinks the body, front and RGB LEDs
void go_back(void); // makes the robot go back to its original position and orientation
void turn_back(int angle_back); //turn to the correct orientation depending on the sign of angle_back
void update_pos(void);// uses the orientation of the robot to increase or decrease the correct position variables


#ifdef __cplusplus
}
#endif

#endif
