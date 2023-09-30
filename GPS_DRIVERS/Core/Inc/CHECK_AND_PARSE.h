#ifndef CHECK_AND_PARSE_H
#define CHECK_AND_PARSE_H

/* The point of CHECK_AND_PARSE is to check validity of data retrieved from GPS receiver */

/* CHECK_SUM is a function used to check the validity of the data that was retrieved from the receiver
 * takes a pointer from the data buffer
 * returns 1 (sets flag) if data is valid
 * returns 0 (turns of flag) if data is invalid
*/
unsigned int CHECK_SUM(void);

/* PARSE_PAYLOAD is a function that parses the payload from the valid data buffer
 * takes a pointer from the data buffer
 * returns a struct of data in decimal form
*/

void PARSE_PAYLOAD(void);
