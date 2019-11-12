/* DTrack2SDK: C++ source file, A.R.T. GmbH
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DTrack2.hpp"

#define DTRACK2_PROT_MAXLEN	DTRACK_PROT_MAXLEN      200  // max length of DTrack2 command string


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
DTrack2::DTrack2(const std::string& server_host, unsigned short server_port, unsigned short data_port,
                 int data_bufsize, int data_timeout_us, int server_timeout_us)
{
	sdk = new DTrackSDK(server_host, server_port, data_port, DTrackSDK::SYS_DTRACK_2,
	                    data_bufsize, data_timeout_us, server_timeout_us);
}


/**
 * 	\brief Destructor.
 */
DTrack2::~DTrack2()
{
	delete sdk;
}


/**
 * 	\brief	Check if initialization was successfull.
 *
 *	@return Valid?
 */
bool DTrack2::valid()
{
	return sdk->isLocalDataPortValid();
}


/**
 * 	\brief Get used UDP port number.
 *
 *	@return	local udp data port used for receiving tracking data
 */
unsigned short DTrack2::get_data_port()
{
	return sdk->getDataPort();
}


/**
 * 	\brief Check last data receive error (timeout)
 *
 *	@return	timeout occured?
 */
bool DTrack2::data_timeout()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_TIMEOUT);
}


/**
 * 	\brief Check last data receive error (net error)
 *
 * 	@return	net error occured?
 */
bool DTrack2::data_neterror()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_NET);
}


/**
 * 	\brief Check last data receive error (parser)
 *
 *	@return	error occured while parsing tracking data?
 */
bool DTrack2::data_parseerror()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_PARSE);
}


/**
 * 	\brief Check if connection to DTrack2 server is completely lost.
 *
 * 	@return connection lost?
 */
bool DTrack2::server_noconnection()
{
	return (!sdk->isCommandInterfaceValid());
}


/**
 * 	\brief	Check last command receive/send error (timeout)
 *
 *	@return	timeout occured?
 */
bool DTrack2::server_timeout()
{
	return (sdk->getLastServerError() == DTrackSDK::ERR_TIMEOUT);
}


/**
 * 	\brief	Check last command receive/send error (network)
 *
 *	@return net error occured?
 */
bool DTrack2::server_neterror()
{
	return (sdk->getLastServerError() == DTrackSDK::ERR_NET);
}


/**
 * 	\brief	Check last command receive/send error (parsing)
 *
 *	@return	error occured while parsing command data?
 */
bool DTrack2::server_parseerror()
{
	return (sdk->getLastServerError() == DTrackSDK::ERR_PARSE);
}


/**
 *	\brief	Receive and process one DTrack data packet (UDP; ASCII protocol)
 *
 *	@return	successful?
 */
bool DTrack2::receive()
{
	if (!sdk->isLocalDataPortValid())
		return false;
	if (!sdk->receive())
		return false;
	// copy body data
	act_num_body = sdk->getNumBody();
	act_body.resize(act_num_body);
	for (int i = 0; i < act_num_body; i++)
	{
		const DTrack_Body_Type_d *dtbt = sdk->getBody(i);
		dtrack2_body_type *dtbt2 = &act_body.at(i);
		dtbt2->id = dtbt->id;
		dtbt2->quality = (float)dtbt->quality;
		for (int j = 0; j < 3; j++)
			dtbt2->loc[j] = (float)dtbt->loc[j];
		for (int j = 0; j < 9; j++)
			dtbt2->rot[j] = (float)dtbt->rot[j];
	}
	// copy flystick data
	act_num_flystick = sdk->getNumFlyStick();
	act_flystick.resize(act_num_flystick);
	for (int i = 0; i < act_num_flystick; i++)
	{
		const DTrack_FlyStick_Type_d *dtfst = sdk->getFlyStick(i);
		dtrack2_flystick_type *dtfst2 = &act_flystick.at(i);
		dtfst2->id = dtfst->id;
		dtfst2->quality = (float)dtfst->quality;
		dtfst2->num_button = dtfst->num_button;
		for (int j = 0; j < DTRACK2_FLYSTICK_MAX_BUTTON; j++)
			dtfst2->button[j] = dtfst->button[j];
		dtfst2->num_joystick = dtfst->num_joystick;
		for (int j = 0; j < DTRACK2_FLYSTICK_MAX_JOYSTICK; j++)
			dtfst2->joystick[j] = (float)dtfst->joystick[j];
		for (int j = 0; j < 3; j++)
			dtfst2->loc[j] = (float)dtfst->loc[j];
		for (int j = 0; j < 9; j++)
			dtfst2->rot[j] = (float)dtfst->rot[j];
	}
	// copy measurement tool data
	act_num_meatool = sdk->getNumMeaTool();
	act_meatool.resize(act_num_meatool);
	for (int i = 0; i < act_num_meatool; i++)
	{
		const DTrack_MeaTool_Type_d *dtmtt = sdk->getMeaTool(i);
		dtrack2_meatool_type *dtmtt2 = &act_meatool.at(i);
		dtmtt2->id = dtmtt->id;
		dtmtt2->quality = (float)dtmtt->quality;
		dtmtt2->num_button = dtmtt->num_button;
		for (int j = 0; j < DTRACK2_MEATOOL_MAX_BUTTON; j++)
			dtmtt2->button[j] = dtmtt->button[j];
		for (int j = 0; j < 3; j++)
			dtmtt2->loc[j] = (float)dtmtt->loc[j];
		for (int j = 0; j < 9; j++)
			dtmtt2->rot[j] = (float)dtmtt->rot[j];
	}
	// copy hand data
	act_num_hand = sdk->getNumHand();
	act_hand.resize(act_num_hand);
	for (int i = 0; i < act_num_hand; i++)
	{
		const DTrack_Hand_Type_d *dtht = sdk->getHand(i);
		dtrack2_hand_type *dtht2 = &act_hand.at(i);
		dtht2->id = dtht->id;
		dtht2->quality = (float)dtht->quality;
		dtht2->lr = dtht->lr;
		dtht2->nfinger = dtht->nfinger;
		for (int j = 0; j < 3; j++)
			dtht2->loc[j] = (float)dtht->loc[j];
		for (int j = 0; j < 9; j++)
			dtht2->rot[j] = (float)dtht->rot[j];
		for (int k = 0; k < DTRACK2_HAND_MAX_FINGER; k++)
		{
			for (int j = 0; j < 3; j++)
				dtht2->finger[k].loc[j] = (float)dtht->finger[k].loc[j];
			for (int j = 0; j < 9; j++)
				dtht2->finger[k].rot[j] = (float)dtht->finger[k].rot[j];
			dtht2->finger[k].radiustip = (float)dtht->finger[k].radiustip;
			for (int j = 0; j < 3; j++)
				dtht2->finger[k].lengthphalanx[j] = (float)dtht->finger[k].lengthphalanx[j];
			for (int j = 0; j < 2; j++)
				dtht2->finger[k].anglephalanx[j] = (float)dtht->finger[k].anglephalanx[j];
		}
	}
	// copy marker data
	act_num_marker = sdk->getNumMarker();
	act_marker.resize(act_num_marker);
	for (int i = 0; i < act_num_marker; i++)
	{
		const DTrack_Marker_Type_d *dtmt = sdk->getMarker(i);
		dtrack2_marker_type *dtmt2 = &act_marker.at(i);
		dtmt2->id = dtmt->id;
		dtmt2->quality = (float)dtmt->quality;
		for (int j = 0; j < 3; j++)
			dtmt2->loc[j] = (float)dtmt->loc[j];
	}
	return true;
}


/**
 * 	\brief	Get frame counter.
 *
 *	Refers to last received frame.
 *	@return	frame counter
 */
unsigned int DTrack2::get_framecounter()
{
	return sdk->getFrameCounter();
}


/**
 * 	\brief	Get timestamp.
 *
 *	Refers to last received frame.
 *	@return	timestamp (-1 if information not available)
 */
double DTrack2::get_timestamp()
{
	return sdk->getTimeStamp();
}


/**
 * 	\brief	Get number of standard bodies.
 *
 *	Refers to last received frame.
 *	@return	number of standard bodies
 */
int DTrack2::get_num_body()
{
	return act_num_body;
}


/**
 * 	\brief	Get 6d data.
 *
 *	Refers to last received frame.
 *	@param[in]	id	id, range 0 .. num_body - 1
 *	@return		id-th standard body data.
 */
dtrack2_body_type DTrack2::get_body(int id)
{
	if (id < act_num_body) {
		return act_body[id];
	}
	dtrack2_body_type dummy;
	memset(&dummy, 0, sizeof(dtrack2_body_type));
	dummy.id = id;
	dummy.quality = -1;
	return dummy;
}


/**
 * 	\brief	Get number of flysticks.
 *
 *	Refers to last received frame.
 *	@return	number of flysticks
 */
int DTrack2::get_num_flystick()
{
	return act_num_flystick;
}


/**
 * 	\brief	Get 6df data.
 *
 *	Refers to last received frame.
 *	@param[in]	id	id, range 0 .. num_flystick - 1
 *	@return		id-th flystick data
 */
dtrack2_flystick_type DTrack2::get_flystick(int id)
{
	if (id < act_num_flystick) {
		return act_flystick[id];
	}
	dtrack2_flystick_type dummy;
	memset(&dummy, 0, sizeof(dtrack2_flystick_type));
	dummy.id = id;
	dummy.quality = -1;
	return dummy;
}


/**
 * 	\brief	Get number of measurement tools.
 *
 *	Refers to last received frame.
 *	@return	number of measurement tools
 */
int DTrack2::get_num_meatool(void)
{
	return act_num_meatool;
}


/**
 * 	\brief	Get 6dmt data.
 *
 *	Refers to last received frame.
 *	@param[in]	id	id, range 0 .. num_meatool - 1
 *	@return		id-th measurement tool data
 */
dtrack2_meatool_type DTrack2::get_meatool(int id)
{
	if (id < act_num_meatool) {
		return act_meatool[id];
	}
	dtrack2_meatool_type dummy;
	memset(&dummy, 0, sizeof(dtrack2_meatool_type));
	dummy.id = id;
	dummy.quality = -1;
	return dummy;
}


/**
 * 	\brief	Get number of fingertracking hands.
 *
 *	Refers to last received frame.
 *	@return	number of fingertracking hands
 */
int DTrack2::get_num_hand()
{
	return act_num_hand;
}


/**
 * 	\brief	Get gl data.
 *
 *	Refers to last received frame.
 *	@param[in]	id	id, range 0 .. num_hand - 1
 *	@return		id-th Fingertracking hand data.
 */
dtrack2_hand_type DTrack2::get_hand(int id)
{
	if (id < act_num_hand) {
		return act_hand[id];
	}
	dtrack2_hand_type dummy;
	memset(&dummy, 0, sizeof(dtrack2_hand_type));
	dummy.id = id;
	dummy.quality = -1;
	return dummy;
}


/**
 * 	\brief	Get number of single markers
 *
 *	Refers to last received frame.
 *	@return	number of single markers
 */
int DTrack2::get_num_marker()
{
	return act_num_marker;
}


/**
 * 	\brief	Get 3d data.
 *
 *	Refers to last received frame.
 *	@param[in]	index	index, range 0 .. num_marker - 1
 *	@return		id-th single marker data
 */
dtrack2_marker_type DTrack2::get_marker(int index)
{
	if (index >= 0 && index < act_num_marker) {
		return act_marker[index];
	}
	dtrack2_marker_type dummy;
	memset(&dummy, 0, sizeof(dtrack2_marker_type));
	dummy.id = 0;
	dummy.quality = -1;
	return dummy;
}


/**
 * 	\brief	Set DTrack2 parameter.
 *
 *	@param[in] 	category	parameter category
 *	@param[in] 	name		parameter name
 *	@param[in] 	value		parameter value
 *	@return 	setting was successful (if not, a DTrack2 error message is available)
 */
bool DTrack2::set_parameter(const std::string category, const std::string name, const std::string value)
{
	return set_parameter(category + " " + name + " " + value);
}


/**
 * 	\brief	Set DTrack2 parameter.
 *
 * 	@param[in]	parameter	total parameter (category, name and value; without starting "dtrack2 set ")
 *	@return		setting was successful (if not, a DTrack2 error message is available)
 */
bool DTrack2::set_parameter(const std::string parameter)
{
	return send_command("set " + parameter);
}


/**
 * 	\brief	Get DTrack2 parameter.
 *
 *	@param[in] 	category	parameter category
 *	@param[in] 	name		parameter name
 *	@param[out]	value		parameter value
 *	@return		getting was successful (if not, a DTrack2 error message is available)
 */
bool DTrack2::get_parameter(const std::string category, const std::string name, std::string& value)
{
	return get_parameter(category + " " + name, value);
}


/**
 * 	\brief	Get DTrack2 parameter.
 *
 *	@param[in] 	parameter	total parameter (category and name; without starting "dtrack2 get ")
 *	@param[out]	value		parameter value
 *	@return		getting was successful (if not, a DTrack2 error message is available)
 */
bool DTrack2::get_parameter(const std::string parameter, std::string& value)
{
	return sdk->getParam(parameter, value);
}

#include <iostream>
/**
 *	\brief	Send DTrack2 command.
 *
 *	@param[in]	command	command (without starting "dtrack2 ")
 *	@return 	command was successful and "dtrack2 ok" is received (if not, a DTrack2 error message is available)
 */
bool DTrack2::send_command(const std::string command)
{
	return (1 == sdk->sendDTrack2Command("dtrack2 " + command, NULL));
}


/**
 * 	\brief	Get last DTrack2 error code.
 *
 *	@param[out]	errorcode	error as code number
 * 	@return 	error code was available, otherwise last command was successful
 */
bool DTrack2::get_lasterror(int& errorcode)
{
	errorcode = sdk->getLastDTrackError();
	return (0 != errorcode);
}


/**
 * 	\brief	Get last DTrack2 error code.
 *
 *	@param[out]	errorstring	error as string
 *	@return 	error code was available, otherwise last command was successful
 */
bool DTrack2::get_lasterror(std::string& errorstring)
{
	if (0 == sdk->getLastDTrackError())
		return false;
	errorstring = sdk->getLastDTrackErrorDescription();
	return true;
}


/**
 * 	\brief	Get DTrack2 message.
 *
 *	@return message was available
 */
bool DTrack2::get_message()
{
	return sdk->getMessage();
}


/**
 * 	\brief Get origin of last DTrack2 message.
 *
 *	@return origin
 */
std::string DTrack2::get_message_origin()
{
	return sdk->getMessageOrigin();
}


/**
 * 	\brief Get status of last DTrack2 message.
 *
 *	@return status
 */
std::string DTrack2::get_message_status()
{
	return sdk->getMessageStatus();
}


/**
 * 	\brief Get frame counter of last DTrack2 message.
 *
 *	@return frame counter
 */
unsigned int DTrack2::get_message_framenr()
{
	return sdk->getMessageFrameNr();
}


/**
 * 	\brief Get error id of last DTrack2 message.
 *
 *	@return error id
 */
unsigned int DTrack2::get_message_errorid()
{
	return sdk->getMessageErrorId();
}


/**
 * 	\brief Get message string of last DTrack2 message.
 *
 *	@return mesage string
 */
std::string DTrack2::get_message_msg()
{
	return sdk->getMessageMsg();
}



