#ifndef MOVE_H_
#define MOVE_H_

#define WHEEL_PERIMETER     		13 // [cm]
#define NSTEP_ONE_TURN    	  	1000 // number of step for 1 turn of the motor
#define SPEED					300 // arbitrary speed

 /**
 * @brief   goes forward for a given distance
 *
 * @param position_l    	 position to reach [cm]
 * @param pos_init    	 position before the action
 */
void forward(float position_l, float pos_init);

/**
* @brief   turn on the right
*
 * @param position_l    	 position to reach [cm]
 * @param pos_init    	 position before the turn
*/
void right(float position_l, float pos_init);

/**
* @brief   turn on the left
*
 * @param position_l    	 position to reach [cm]
 * @param pos_init    	 position before the turn
*/
void left(float position_l, float pos_init);

#endif /* MOVE_H_ */
