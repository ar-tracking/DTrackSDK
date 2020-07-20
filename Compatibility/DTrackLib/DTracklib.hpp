/* dtracklib: C++ header file, A.R.T. GmbH
 *
 * dtracklib: functions to receive and process DTrack UDP packets (ASCII protocol)
 *
 * Copyright (c) 2000-2017, Advanced Realtime Tracking GmbH
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Version v2.5.0
 *
 */

#ifndef _ART_DTRACKLIB_H
#define _ART_DTRACKLIB_H

#include <vector>

#include "../../include/DTrackSDK.hpp"

// Typedefs

/**
 * 	\brief	Single marker data (3DOF, float)
 */
typedef struct{
	int id;          //!< id number (starting with 1)
	float quality;   //!< quality (0.0 <= qu <= 1.0; -1 not tracked)
	float loc[3];    //!< location (in mm)
} dtracklib_marker_type;

/**
 * 	\brief	Standard body data (6DOF, float)
 *
 *	Currently not tracked bodies get a quality of -1.
 */
typedef struct{
	unsigned long id;	//!< id number
	float quality;		//!< quality (0 <= qu <= 1)
	float loc[3];		//!< location (in mm)
	float ang[3];		//!< orientation angles (eta, theta, phi; in deg); not used
	float rot[9];		//!< rotation matrix (column-wise)
} dtracklib_body_type;

#define DTRACKLIB_HAND_MAX_FINGER    5  //!< A.R.T. Fingertracking hand data: maximum number of fingers

/**
 *	\brief	A.R.T. Fingertracking hand data (6DOF + fingers, double)
 *
 *	Currently not tracked bodies get a quality of -1.
 */
typedef struct{
	int id;         //!< id number (starting with 0)
	float quality;  //!< quality (0 <= qu <= 1, no tracking if -1)
	int lr;         //!< left (0) or right (1) hand
	int nfinger;    //!< number of fingers (maximum 5)
	float loc[3];   //!< back of the hand: location (in mm)
	float rot[9];   //!< back of the hand: rotation matrix (column-wise)
	struct{
		float loc[3];            //!< location (in mm)
		float rot[9];            //!< rotation matrix (column-wise)
		float radiustip;         //!< radius of tip
		float lengthphalanx[3];  //!< length of phalanxes; order: outermost, middle, innermost
		float anglephalanx[2];   //!< angle between phalanxes
	} finger[DTRACKLIB_HAND_MAX_FINGER];	//!< order: thumb, index finger, middle finger, ...
} dtracklib_glove_type;

#define DTRACKLIB_FLYSTICK_MAX_BUTTON    16	//!< A.R.T. FlyStick data: maximum number of buttons

/**
 * 	\brief	A.R.T. Flystick data (6DOF + buttons, float)
 *
 * 	Currently not tracked bodies get a quality of -1.
 *	Note the maximum number of buttons and joystick values.
 */
typedef struct{
	unsigned long id;	//!< id number
	float quality;		//!< quality (0 <= qu <= 1, no tracking if -1)
	unsigned long bt;	//!< pressed buttons (binary coded)
	float loc[3];		//!< location (in mm)
	float ang[3];		//!< orientation angles (eta, theta, phi; in deg); not used
	float rot[9];		//!< rotation matrix (column-wise)
} dtracklib_flystick_type;

/**
 * 	\brief	Measurement tool data (6DOF + buttons, float)
 *
 * 	Currently not tracked bodies get a quality of -1.
 * 	Note the maximum number of buttons.
 */
typedef struct{
	unsigned long id;    //!< id number
	float quality;        //!< quality (0 <= qu <= 1, no tracking if -1)
	unsigned long bt;    //!< pressed buttons (binary coded)
	float loc[3];         //!< location (in mm)
	float rot[9];         //!< rotation matrix (column-wise)
} dtracklib_meatool_type;

// DTrack remote commands:
#define DTRACKLIB_CMD_CAMERAS_OFF           0x1000
#define DTRACKLIB_CMD_CAMERAS_ON            0x1001
#define DTRACKLIB_CMD_CAMERAS_AND_CALC_ON   0x1003

#define DTRACKLIB_CMD_SEND_DATA             0x3100
#define DTRACKLIB_CMD_STOP_DATA             0x3200
#define DTRACKLIB_CMD_SEND_N_DATA           0x3300

/**
 * 	\brief Wrapper class for DTracklib SDK.
 */
class DTracklib
{
public:
	/**
	 * 	\brief	Constructor.
	 *
	 * @param[in]	udpport			UDP port number to receive data from DTrack
	 * @param[in]	remote_ip		DTrack remote control: ip address of DTrack PC (NULL if not used)
	 * @param[in]	remote_port 	port number of DTrack remote control (0 if not used)
	 * @param[in]	udpbufsize		size of buffer for UDP packets (in bytes)
	 * @param[in]	udptimeout_us	UDP timeout (receiving and sending) in us (micro sec)
	 */
	DTracklib(
	        unsigned short udpport = 5000, char* remote_ip = NULL, unsigned short remote_port = 0,
	        int udpbufsize = 10000, unsigned long udptimeout_us = 1000000
	                                                              );
	
	/**
	 * \brief Destructor.
	 */
	~DTracklib(void);
	
	/**
	 * 	\brief	Check if initialization was successfull.
	 *
	 *	@return Valid?
	 */
	bool valid(void);
	
	/**
	 * 	\brief Check last receive/send error (timeout)
	 *
	 *	@return	timeout occured?
	 */
	bool timeout();
	
	/**
	 * 	\brief Check last receive/send error (udp error)
	 *
	 * 	@return	udp error occured?
	 */
	bool udperror();
	
	/**
	 * 	\brief Check last receive/send error (parser)
	 *
	 *	@return	error occured while parsing tracking data?
	 */
	bool parseerror();
	
	/**
	 *	\brief	Receive and process one DTrack data packet (UDP; ASCII protocol)
	 *
	 *	@return	successful?
	 */
	bool receive();
	
	/**
	 * 	\brief	Get frame counter.
	 *
	 *	Refers to last received frame.
	 *	@return	frame counter
	 */
	unsigned long get_framenr();
	
	/**
	 * 	\brief	Get timestamp.
	 *
	 *	Refers to last received frame.
	 *	@return	timestamp (-1 if information not available)
	 */
	double get_timestamp();
	
	/**
	 * 	\brief	Get number of calibrated bodies.
	 *
	 *	@return	number of calibrated bodies; -1, if information not available
	 */
	int get_nbodycal();
	
	/**
	 * 	\brief	Get number of standard bodies.
	 *
	 *	Refers to last received frame.
	 *	@return	number of standard bodies
	 */
	int get_nbody();
	
	/**
	 * 	\brief	Get 6d data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	id	id, range 0 .. nbody - 1
	 *	@return		id-th standard body data.
	 */
	dtracklib_body_type get_body(int id);
	
	/**
	 * 	\brief	Get number of flysticks.
	 *
	 *	Refers to last received frame.
	 *	@return	number of flysticks
	 */
	int get_nflystick();
	
	/**
	 * 	\brief	Get 6df data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	id	id, range 0 .. nflystick - 1
	 *	@return		id-th flystick data
	 */
	dtracklib_flystick_type get_flystick(int id);
	
	/**
	 * 	\brief	Get number of measurement tools.
	 *
	 *	Refers to last received frame.
	 *	@return	number of measurement tools
	 */
	int get_nmeatool();
	
	/**
	 * 	\brief	Get 6dmt data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	id	id, range 0 .. nmeatool - 1
	 *	@return		id-th measurement tool data
	 */
	dtracklib_meatool_type get_meatool(int id);
	
	/**
	 * 	\brief	Get number of fingertracking hands.
	 *
	 *	Refers to last received frame.
	 *	@return	number of fingertracking hands
	 */
	int get_nglove();
	
	/**
	 * 	\brief	Get gl data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	id	id, range 0 .. nglove - 1
	 *	@return		id-th Fingertracking hand data.
	 */
	dtracklib_glove_type get_glove(int id);
	
	/**
	 * 	\brief	Get number of single markers
	 *
	 *	Refers to last received frame.
	 *	@return	number of single markers
	 */
	int get_nmarker();
	
	/**
	 * 	\brief	Get 3d data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	index	index, range 0 .. nmarker - 1
	 *	@return		i-th single marker data
	 */
	dtracklib_marker_type get_marker(int index);
	
	/**
	 *	\brief	Send one remote control command (UDP; ASCII protocol) to DTrack.
	 *
	 *	@param[in]	cmd		command code
	 *	@param[in]	val		additional value (depends on command code), default 0
	 *
	 *	@return sending was successful
	 */
	bool send(unsigned short cmd, int val = 0);
	
private:
	DTrackSDK *sdk;                                     //!< unified sdk
	int act_nbodycal;                                   //!< number of calibrated bodies (-1, if information not available)
	int act_nbody;                                      //!< number of standard bodies
	std::vector<dtracklib_body_type> act_body;          //!< array containing 6d data
	int act_nflystick;                                  //!< number of flysticks
	std::vector<dtracklib_flystick_type> act_flystick;  //!< array containing 6df data
	int act_nmeatool;                                   //!< number of measurement tools
	std::vector<dtracklib_meatool_type> act_meatool;    //!< array containing 6dmt data
	int act_nmarker;                                    //!< number of single markers
	std::vector<dtracklib_marker_type> act_marker;      //!< array containing 3d data
	int act_nglove;                                     //!< number of DataGloves
	std::vector<dtracklib_glove_type> act_glove;        //!< array containing gl data
};

#endif // _ART_DTRACKLIB_H
