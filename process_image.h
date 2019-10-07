#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

/*@brief extract the number of black lines in front of the camera
 *
 * @param  *buffer  is the image buffer from which we extract the line number
 *
 * @return  the number of black lines extracted
 */
uint16_t extract_line_nb(uint8_t *buffer);

/*@brief gives access to another file, to the line number found in the image
 *
 * @return the line number found in the image
 */
uint16_t get_line_nb(void);

/*@brief initializes the necessary threads for the reading and processing of the image from the camera
 *
 */
void process_image_start(void);

#endif /* PROCESS_IMAGE_H */
