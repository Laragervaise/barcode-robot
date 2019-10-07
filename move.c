#include "move.h"
#include "motors.h"

void forward(float position_l, float pos_init)
{
	// as long as the position desired isn't reached, the motors run
	// conversion from steps to cm in order to do the differential comparaison
	while(left_motor_get_pos() - pos_init <= position_l * NSTEP_ONE_TURN / WHEEL_PERIMETER){
    		right_motor_set_speed(SPEED);
    		left_motor_set_speed(SPEED);
    	}
    	left_motor_set_speed(0);
    	right_motor_set_speed(0);
 }

void right(float position_l, float pos_init)
{
	while(left_motor_get_pos() - pos_init <= position_l * NSTEP_ONE_TURN / WHEEL_PERIMETER){
		right_motor_set_speed(-SPEED);
		left_motor_set_speed(SPEED);
	}
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}

void left(float position_l, float pos_init)
{
	while(pos_init - left_motor_get_pos() <= position_l * NSTEP_ONE_TURN / WHEEL_PERIMETER){
		right_motor_set_speed(SPEED);
		left_motor_set_speed(-SPEED);
	}
	left_motor_set_speed(0);
	right_motor_set_speed(0);
}
