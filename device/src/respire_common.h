/******************** (C) COPYRIGHT 2011 Specknet.org **************************
* File Name          : respire_common.h
* Author             : Cosmin Dumitrache, Janek Mann
* Version            : V0.1
* Date               : 17-October-2011
* Description        : Respire common configuration file to be used by both	the
                       base station and the respire nodes.
*******************************************************************************/

/*
 * Protocol overview:
 * 	1. Respire node sends first packet of type RESPIRE_MSG_ACCEL
 *  2. Base station receives packet and grabs RTC_COUNTER and the
 *     2 accelerometer measurements.
 *  3. Respire node sends 6 more continuation packets of type
 *     RESPIRE_MSG_ACCEL_CONT containing 5 measurements each.
 *  4. Base station receives the 6 packets and grabs the measurement values,
 *     totaling 32 accelereomter values when the last packet is received.
 *  5. If this is the first packet ignore the values
 *     else grab the values, compute time difference, and then go to step 1.
 */

/* 
 * Accelerometer packet
 * 
 * TYPE 	   = The type of the message being sent (see definitions below)
 * ID 		   = The node ID of the Respire node that sent the packet
 * RTC_COUNTER = The value of the RTC counter taken on the respire node at the
 *               end of the 32 accelerometer measurements.
 * UNDEFINED   = This 14B part of the packet is not used in current protocol.
 * X, Y, Z	   = An accelerometer measurement, where each vector component is
 *               stored in 2B.
 *
 *   1B	  1B	      4B		14B (UNDEFINED)		    6B 		  					  6B
 * +----+----+----+----+----+----+-     -+----+----+----+----+----+----+----+----+----+----+----+----+
 * |TYPE| ID |    RTC_COUNTER	 | . . . |    X         Y	      Z    |    X         Y	        Z	 |
 * +----+----+----+----+----+----+-     -+----+----+----+----+----+----+----+----+----+----+----+----+
 */

/* 
 * Accelerometer continuation packet
 *
 * TYPE 	   = The type of the message being sent (see definitions below)
 * ID 		   = The node ID of the Respire node that sent the packet
 * X, Y, Z	   = An accelerometer measurement, where each vector component is
 *               stored in 2B.
 *
 *   1B	  1B	          5 X 6B . . .		     		 		  
 * +----+----+----+----+----+----+----+----+	-+
 * |TYPE| ID |    X         Y	      Z	   |. . .|
 * +----+----+----+----+----+----+----+----+	-+
 */

//Communication protocol message types
#define RESPIRE_MSG_RESERVED 		0
#define	RESPIRE_MSG_ACCEL			1
#define RESPIRE_MSG_ACCEL_CONT		2
#define RESPIRE_MSG_PRESS			3
#define RESPIRE_MSG_PRESS_CONT		4
#define	RESPIRE_MSG_ACCEL_DEMO			5
#define RESPIRE_MSG_PRESS_DEMO			6

/******************* (C) COPYRIGHT 2011 Specknet.org ************END OF FILE****/
