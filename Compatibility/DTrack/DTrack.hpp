/* DTrackSDK: C++ header file, A.R.T. GmbH
 *
 * DTrack: functions to receive and process DTrack UDP packets (ASCII protocol)
 *
 * Copyright (c) 2005-2017, Advanced Realtime Tracking GmbH
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
 * Purpose:
 *  - receives DTrack UDP packets (ASCII protocol) and converts them into easier to handle data
 *  - sends DTrack remote commands (UDP)
 *  - DTrack network protocol due to: 'Technical Appendix DTrack v1.24 (December 19, 2006)'
 *  - for DTrack versions v1.16 - v1.24 (and compatible versions)
 *
 * Usage:
 *  - for Linux, Unix:
 *    - comment '#define OS_WIN', uncomment '#define OS_UNIX' in file 'DTrack.cpp'
 *  - for MS Windows:
 *    - comment '#define OS_UNIX', uncomment '#define OS_WIN' in file 'DTrack.cpp'
 *    - link with 'ws2_32.lib'
 *
 */

#ifndef _ART_DTRACK_H
#define _ART_DTRACK_H

#include "../../include/DTrackSDK.hpp"

// Typedefs

/**
 * 	\brief	Standard body data (6DOF, float)
 *
 *	Currently not tracked bodies get a quality of -1.
 */
typedef struct{
	int id;          //!< id number (starting with 0)
	float quality;   //!< quality (0 <= qu <= 1, no tracking if -1)
	float loc[3];    //!< location (in mm)
	float rot[9];    //!< rotation matrix (column-wise)
} dtrack_body_type;

#define DTRACK_FLYSTICK_MAX_BUTTON    16	//!< A.R.T. FlyStick data: maximum number of buttons
#define DTRACK_FLYSTICK_MAX_JOYSTICK   8	//!< A.R.T. FlyStick data: maximum number of joystick values

/**
 * 	\brief	A.R.T. Flystick data (6DOF + buttons, float)
 *
 * 	Currently not tracked bodies get a quality of -1.
 *	Note the maximum number of buttons and joystick values.
 */
typedef struct{
	int id;         //!< id number (starting with 0)
	float quality;  //!< quality (0 <= qu <= 1, no tracking if -1)
	int num_button; //!< number of buttons
	int button[DTRACK_FLYSTICK_MAX_BUTTON];  //!< button state (1 pressed, 0 not pressed); 0 front, 1..n-1 right to left
	int num_joystick;  //!< number of joystick values
	float joystick[DTRACK_FLYSTICK_MAX_JOYSTICK];  //!< joystick value (-1 <= joystick <= 1); 0 horizontal, 1 vertical
	float loc[3];   //!< location (in mm)
	float rot[9];   //!< rotation matrix (column-wise)
} dtrack_flystick_type;

#define DTRACK_MEATOOL_MAX_BUTTON    1  //!< Measurement tool data: maximum number of buttons

/**
 * 	\brief	Measurement tool data (6DOF + buttons, float)
 *
 * 	Currently not tracked bodies get a quality of -1.
 * 	Note the maximum number of buttons.
 */
typedef struct{
	int id;         //!< id number (starting with 0)
	float quality;  //!< quality (0 <= qu <= 1, no tracking if -1)
	int num_button; //!< number of buttons
	int button[DTRACK_MEATOOL_MAX_BUTTON];  //!< button state (1 pressed, 0 not pressed): 0 front, 1..n-1 right to left
	float loc[3];   //!< location (in mm)
	float rot[9];   //!< rotation matrix (column-wise)
} dtrack_meatool_type;

#define DTRACK_HAND_MAX_FINGER    5  //!< A.R.T. Fingertracking hand data: maximum number of fingers

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
	} finger[DTRACK_HAND_MAX_FINGER];	//!< order: thumb, index finger, middle finger, ...
} dtrack_hand_type;

/**
 * 	\brief	Single marker data (3DOF, float)
 */
typedef struct{
	int id;          //!< id number (starting with 1)
	float quality;   //!< quality (0.0 <= qu <= 1.0; -1 not tracked)
	float loc[3];    //!< location (in mm)
} dtrack_marker_type;

// DTrack remote commands:
#define DTRACK_CMD_CAMERAS_OFF			1
#define DTRACK_CMD_CAMERAS_ON			2
#define DTRACK_CMD_CAMERAS_AND_CALC_ON	3

#define DTRACK_CMD_SEND_DATA			11
#define DTRACK_CMD_STOP_DATA			12
#define DTRACK_CMD_SEND_N_DATA			13

/**
 * 	Wrapper class for DTrack SDK.
 */
class DTrack
{
public:
	/**
	 * 	\brief	Constructor.
	 *
	 * 	@param[in] data_port		UDP port number to receive data from DTrack
	 *	@param[in] remote_host		DTrack remote control: hostname or IP address of DTrack PC (NULL if not used)
	 *	@param[in] remote_port		port number of DTrack remote control (0 if not used)
	 *	@param[in] data_bufsize		size of buffer for UDP packets (in bytes)
	 *	@param[in] data_timeout_us	UDP timeout (receiving and sending) in us (micro second)
	 */
	DTrack(
	        int data_port = 5000, const char* remote_host = NULL, int remote_port = 0,
	        int data_bufsize = 20000, int data_timeout_us = 1000000
	                                                        );
	
	/**
	 * 	\brief	Destructor.
	 */
	~DTrack(void);
	
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
	unsigned int get_framecounter();
	
	/**
	 * 	\brief	Get timestamp.
	 *
	 *	Refers to last received frame.
	 *	@return	timestamp (-1 if information not available)
	 */
	double get_timestamp();
	
	/**
	 * 	\brief	Get number of standard bodies.
	 *
	 *	Refers to last received frame.
	 *	@return	number of standard bodies
	 */
	int get_num_body();
	
	/**
	 * 	\brief	Get 6d data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	id	id, range 0 .. num_body - 1
	 *	@return		id-th standard body data.
	 */
	dtrack_body_type get_body(int id);
	
	/**
	 * 	\brief	Get number of flysticks.
	 *
	 *	Refers to last received frame.
	 *	@return	number of flysticks
	 */
	int get_num_flystick();
	
	/**
	 * 	\brief	Get 6df data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	id	id, range 0 .. num_flystick - 1
	 *	@return		id-th flystick data
	 */
	dtrack_flystick_type get_flystick(int id);
	
	/**
	 * 	\brief	Get number of measurement tools.
	 *
	 *	Refers to last received frame.
	 *	@return	number of measurement tools
	 */
	int get_num_meatool();
	
	/**
	 * 	\brief	Get 6dmt data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	id	id, range 0 .. num_meatool - 1
	 *	@return		id-th measurement tool data
	 */
	dtrack_meatool_type get_meatool(int id);
	
	/**
	 * 	\brief	Get number of fingertracking hands.
	 *
	 *	Refers to last received frame.
	 *	@return	number of fingertracking hands
	 */
	int get_num_hand();
	
	/**
	 * 	\brief	Get gl data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	id	id, range 0 .. num_hand - 1
	 *	@return		id-th Fingertracking hand data.
	 */
	dtrack_hand_type get_hand(int id);
	
	/**
	 * 	\brief	Get number of single markers
	 *
	 *	Refers to last received frame.
	 *	@return	number of single markers
	 */
	int get_num_marker();
	
	/**
	 * 	\brief	Get 3d data.
	 *
	 *	Refers to last received frame.
	 *	@param[in]	index	index, range 0 .. num_marker - 1
	 *	@return		id-th single marker data
	 */
	dtrack_marker_type get_marker(int index);
	
	/**
	 * 	\brief	Control cameras by remote commands to DTrack (UDP; ASCII protocol; Default off).
	 *
	 *	@param[in]	onoff	switch on/off
	 *	@return		sending of remote commands was successfull
	 */
	bool cmd_cameras(bool onoff);
	
	/**
	 * 	\brief	Control tracking calculation by remote commands to DTrack (UDP, ASCII protocol; Default: on).
	 *
	 *	@param[in]	onoff	Switch function (1 on, 0 off)
	 *	@return 	sending of remote commands was successfull (1 yes, 0 no)
	 */
	bool cmd_tracking(bool onoff);
	
	/**
	 * 	\brief	Control sending of UDP output data by remote commands to DTrack (UDP, ASCII protocol; Default: on).
	 *
	 *	@param[in]	onoff	Switch function (1 on, 0 off)
	 *	@return 	sending of remote commands was successfull (1 yes, 0 no)
	 */
	bool cmd_sending_data(bool onoff);
	
	/**
	 * 	\brief	Control sending of a fixed number of UDP output data frames by remote commands to DTrack (UDP, ASCII protocol).
	 *
	 *	@param[in]	frames	Number of frames
	 *	@return 	sending of remote commands was successfull (1 yes, 0 no)
	 */
	bool cmd_sending_fixed_data(int frames);  // start sending of a fixed number of UDP output frames
	
private:
	DTrackSDK *sdk;         //!< unified sdk
	bool remoteCameras;		//!< DTrack status: cameras on/off
	bool remoteTracking;	//!< DTrack status: tracking on/off
	bool remoteSending;		//!< DTrack status: sending of UDP output data on/off
	int act_num_body;								//!< number of standard bodies
	std::vector<dtrack_body_type> act_body;			//!< array containing 6d data
	int act_num_flystick;							//!< number of flysticks
	std::vector<dtrack_flystick_type> act_flystick;	//!< array containing 6df data
	int act_num_meatool;							//!< number of measurement tools
	std::vector<dtrack_meatool_type> act_meatool;	//!< array containing 6dmt data
	int act_num_marker;								//!< number of single markers
	std::vector<dtrack_marker_type> act_marker;		//!< array containing 3d data
	int act_num_hand;								//!< number of fingertracking hands
	std::vector<dtrack_hand_type> act_hand;			//!< array containing gl data
};

#endif // _ART_DTRACK_H
