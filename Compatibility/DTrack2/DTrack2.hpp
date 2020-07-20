/* DTrack2SDK: C++ header file, A.R.T. GmbH
 *
 * DTrack2: functions to receive and process DTrack UDP packets (ASCII protocol), as
 * well as to exchange DTrack2 TCP command strings.
 *
 * Copyright (c) 2007-2017, Advanced Realtime Tracking GmbH
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
 *  - sends and receives DTrack2 commands (TCP)
 *  - DTrack2 network protocol due to: 'Technical Appendix DTrack v2.0'
 *  - for ARTtrack Controller versions v0.2 (and compatible versions)
 *
 * Usage:
 *  - for Linux, Unix:
 *    - comment '#define OS_WIN', uncomment '#define OS_UNIX' in file 'DTrack2.cpp'
 *  - for MS Windows:
 *    - comment '#define OS_UNIX', uncomment '#define OS_WIN' in file 'DTrack2.cpp'
 *    - link with 'ws2_32.lib'
 *
 */

#ifndef _ART_DTRACK2_H
#define _ART_DTRACK2_H

#include <string>
#include <vector>

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
} dtrack2_body_type;

#define DTRACK2_FLYSTICK_MAX_BUTTON    16	//!< A.R.T. FlyStick data: maximum number of buttons
#define DTRACK2_FLYSTICK_MAX_JOYSTICK   8	//!< A.R.T. FlyStick data: maximum number of joystick values

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
	int button[DTRACK2_FLYSTICK_MAX_BUTTON];  //!< button state (1 pressed, 0 not pressed); 0 front, 1..n-1 right to left
	int num_joystick;  //!< number of joystick values
	float joystick[DTRACK2_FLYSTICK_MAX_JOYSTICK];  //!< joystick value (-1 <= joystick <= 1); 0 horizontal, 1 vertical
	float loc[3];   //!< location (in mm)
	float rot[9];   //!< rotation matrix (column-wise)
} dtrack2_flystick_type;

#define DTRACK2_MEATOOL_MAX_BUTTON    1  //!< Measurement tool data: maximum number of buttons

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
	int button[DTRACK2_MEATOOL_MAX_BUTTON];  //!< button state (1 pressed, 0 not pressed): 0 front, 1..n-1 right to left
	float loc[3];   //!< location (in mm)
	float rot[9];   //!< rotation matrix (column-wise)
} dtrack2_meatool_type;

#define DTRACK2_HAND_MAX_FINGER    5  //!< A.R.T. Fingertracking hand data: maximum number of fingers

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
	} finger[DTRACK2_HAND_MAX_FINGER];	//!< order: thumb, index finger, middle finger, ...
} dtrack2_hand_type;

/**
 * 	\brief	Single marker data (3DOF, float)
 */
typedef struct{
	int id;          //!< id number (starting with 1)
	float quality;   //!< quality (0.0 <= qu <= 1.0; -1 not tracked)
	float loc[3];    //!< location (in mm)
} dtrack2_marker_type;

/**
 * 	\brief Wrapper class for DTrack 2 SDK.
 */
class DTrack2
{
public:
	
	/**
	 * 	\brief	Constructor.
	 *
	 * 	@param[in] server_host 			TCP access to DTrack2 server: hostname or IP address of ARTtrack Controller, empty string if not used
	 *	@param[in] server_port			TCP access to DTrack2 server: port number of ARTtrack Controller, default 50105
	 *	@param[in] data_port			UDP port number to receive tracking data from ARTtrack Controller (0 if to be chosen)
	 *	@param[in] data_bufsize			size of buffer for UDP packets (in bytes)
	 *	@param[in] data_timeout_us		UDP timeout (receiving) in us (in micro second)
	 *	@param[in] server_timeout_us	TCP timeout for access to DTrack2 server (in micro second; receiving and sending)
	 */
	DTrack2(
	        const std::string& server_host = "", unsigned short server_port = 50105, unsigned short data_port = 0,
	        int data_bufsize = 20000, int data_timeout_us = 1000000, int server_timeout_us = 10000000
	                                                                                         );
	
	/**
	 * 	\brief Destructor.
	 */
	~DTrack2();
	
	/**
	 * 	\brief	Check if initialization was successfull.
	 *
	 *	@return Valid?
	 */
	bool valid();
	
	/**
	 * 	\brief Get used UDP port number.
	 *
	 *	@return	local udp data port used for receiving tracking data
	 */
	unsigned short get_data_port();
	
	/**
	 * 	\brief Check last data receive error (timeout)
	 *
	 *	@return	timeout occured?
	 */
	bool data_timeout();
	
	/**
	 * 	\brief Check last data receive error (net error)
	 *
	 * 	@return	net error occured?
	 */
	bool data_neterror();
	
	/**
	 * 	\brief Check last data receive error (parser)
	 *
	 *	@return	error occured while parsing tracking data?
	 */
	bool data_parseerror();
	
	/**
	 * 	\brief Check if connection to DTrack2 server is completely lost.
	 *
	 * 	@return connection lost?
	 */
	bool server_noconnection();
	
	/**
	 * 	\brief	Check last command receive/send error (timeout)
	 *
	 *	@return	timeout occured?
	 */
	bool server_timeout();
	
	/**
	 * 	\brief	Check last command receive/send error (network)
	 *
	 *	@return net error occured?
	 */
	bool server_neterror();
	
	/**
	 * 	\brief	Check last command receive/send error (parsing)
	 *
	 *	@return	error occured while parsing command data?
	 */
	bool server_parseerror();
	
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
	dtrack2_body_type get_body(int id);
	
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
	dtrack2_flystick_type get_flystick(int id);
	
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
	dtrack2_meatool_type get_meatool(int id);
	
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
	dtrack2_hand_type get_hand(int id);
	
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
	dtrack2_marker_type get_marker(int index);
	
	/**
	 * 	\brief	Set DTrack2 parameter.
	 *
	 *	@param[in] 	category	parameter category
	 *	@param[in] 	name		parameter name
	 *	@param[in] 	value		parameter value
	 *	@return 	setting was successful (if not, a DTrack2 error message is available)
	 */
	bool set_parameter(const std::string category, const std::string name, const std::string value);
	
	/**
	 * 	\brief	Set DTrack2 parameter.
	 *
	 * 	@param[in]	parameter	total parameter (category, name and value; without starting "dtrack2 set ")
	 *	@return		setting was successful (if not, a DTrack2 error message is available)
	 */
	bool set_parameter(const std::string parameter);
	
	/**
	 * 	\brief	Get DTrack2 parameter.
	 *
	 *	@param[in] 	category	parameter category
	 *	@param[in] 	name		parameter name
	 *	@param[out]	value		parameter value
	 *	@return		getting was successful (if not, a DTrack2 error message is available)
	 */
	bool get_parameter(const std::string category, const std::string name, std::string& value);
	
	/**
	 * 	\brief	Get DTrack2 parameter.
	 *
	 *	@param[in] 	parameter	total parameter (category and name; without starting "dtrack2 get ")
	 *	@param[out]	value		parameter value
	 *	@return		getting was successful (if not, a DTrack2 error message is available)
	 */
	bool get_parameter(const std::string parameter, std::string& value);
	
	/**
	 *	\brief	Send DTrack2 command.
	 *
	 *	@param[in]	command	command (without starting "dtrack2 ")
	 *	@return 	command was successful and "dtrack2 ok" is received (if not, a DTrack2 error message is available)
	 */
	bool send_command(const std::string command);
	
	/**
	 * 	\brief	Get last DTrack2 error code.
	 *
	 *	@param[out]	errorcode	error as code number
	 * 	@return 	error code was available, otherwise last command was successful
	 */
	bool get_lasterror(int& errorcode);
	
	/**
	 * 	\brief	Get last DTrack2 error code.
	 *
	 *	@param[out]	errorstring	error as string
	 *	@return 	error code was available, otherwise last command was successful
	 */
	bool get_lasterror(std::string& errorstring);
	
	/**
	 * 	\brief	Get DTrack2 message.
	 *
	 *	@return message was available
	 */
	bool get_message();
	
	/**
	 * 	\brief Get origin of last DTrack2 message.
	 *
	 *	@return origin
	 */
	std::string get_message_origin();
	
	/**
	 * 	\brief Get status of last DTrack2 message.
	 *
	 *	@return status
	 */
	std::string get_message_status();
	
	/**
	 * 	\brief Get frame counter of last DTrack2 message.
	 *
	 *	@return frame counter
	 */
	unsigned int get_message_framenr();
	
	/**
	 * 	\brief Get error id of last DTrack2 message.
	 *
	 *	@return error id
	 */
	unsigned int get_message_errorid();
	
	/**
	 * 	\brief Get message string of last DTrack2 message.
	 *
	 *	@return mesage string
	 */
	std::string get_message_msg();
private:
	DTrackSDK *sdk;                                     //!< unified sdk
	int act_num_body;									//!< number of standard bodies
	std::vector<dtrack2_body_type> act_body;			//!< array containing 6df data
	int act_num_flystick;								//!< number of flysticks
	std::vector<dtrack2_flystick_type> act_flystick;	//!< array containing flystick data
	int act_num_meatool;								//!< number of measurement tools
	std::vector<dtrack2_meatool_type> act_meatool;		//!< array containing 6dmt data
	int act_num_marker;									//!< number of single markers
	std::vector<dtrack2_marker_type> act_marker;		//!< array containing 3d data
	int act_num_hand;									//!< number of fingertracking hands
	std::vector<dtrack2_hand_type> act_hand;			//!< array containing gl data
};

#endif // _ART_DTRACK2_H


