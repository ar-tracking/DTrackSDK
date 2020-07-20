/* DTrackSDK: C++ source file, A.R.T. GmbH
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
 */

#include "DTrack.hpp"

#include <string.h>
#include <sstream>

/**
 * 	\brief	Constructor.
 *
 * 	@param[in] data_port		UDP port number to receive data from DTrack
 *	@param[in] remote_host		DTrack remote control: hostname or IP address of DTrack PC (NULL if not used)
 *	@param[in] remote_port		port number of DTrack remote control (0 if not used)
 *	@param[in] data_bufsize		size of buffer for UDP packets (in bytes)
 *	@param[in] data_timeout_us	UDP timeout (receiving and sending) in us (micro second)
 */
DTrack::DTrack(
        int data_port, const char* remote_host, int remote_port,
        int data_bufsize, int data_timeout_us
        )
{
	std::string s = "";
	if (remote_host != NULL)
		s = remote_host;
	remoteCameras = false;
	remoteTracking = true;
	remoteSending = true;
	sdk = new DTrackSDK(s, remote_port, data_port, DTrackSDK::SYS_DTRACK_UNKNOWN, data_bufsize, data_timeout_us, data_timeout_us);
}


/**
 * 	\brief	Destructor.
 */
DTrack::~DTrack(void)
{
	delete sdk;
}


/**
 * 	\brief	Check if initialization was successfull.
 *
 *	@return Valid?
 */
bool DTrack::valid()
{
	return sdk->isLocalDataPortValid();
}


/**
 * 	\brief Check last receive/send error (timeout)
 *
 *	@return	timeout occured?
 */
bool DTrack::timeout()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_TIMEOUT);
}


/**
 * 	\brief Check last receive/send error (udp error)
 *
 * 	@return	udp error occured?
 */
bool DTrack::udperror()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_NET);
}


/**
 * 	\brief Check last receive/send error (parser)
 *
 *	@return	error occured while parsing tracking data?
 */
bool DTrack::parseerror()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_PARSE);
}


/**
 *	\brief	Receive and process one DTrack data packet (UDP; ASCII protocol)
 *
 *	@return	successful?
 */
bool DTrack::receive()
{
	if (!sdk->receive())
		return false;
	// copy body data
	act_num_body = sdk->getNumBody();
	act_body.resize(act_num_body);
	for (int i = 0; i < act_num_body; i++)
	{
		const DTrack_Body_Type_d *dtbt = sdk->getBody(i);
		dtrack_body_type *dtbt2 = &act_body.at(i);
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
		dtrack_flystick_type *dtfst2 = &act_flystick.at(i);
		dtfst2->id = dtfst->id;
		dtfst2->quality = (float)dtfst->quality;
		dtfst2->num_button = dtfst->num_button;
		for (int j = 0; j < DTRACK_FLYSTICK_MAX_BUTTON; j++)
			dtfst2->button[j] = dtfst->button[j];
		dtfst2->num_joystick = dtfst->num_joystick;
		for (int j = 0; j < DTRACK_FLYSTICK_MAX_JOYSTICK; j++)
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
		dtrack_meatool_type *dtmtt2 = &act_meatool.at(i);
		dtmtt2->id = dtmtt->id;
		dtmtt2->quality = (float)dtmtt->quality;
		dtmtt2->num_button = dtmtt->num_button;
		for (int j = 0; j < DTRACK_MEATOOL_MAX_BUTTON; j++)
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
		dtrack_hand_type *dtht2 = &act_hand.at(i);
		dtht2->id = dtht->id;
		dtht2->quality = (float)dtht->quality;
		dtht2->lr = dtht->lr;
		dtht2->nfinger = dtht->nfinger;
		for (int j = 0; j < 3; j++)
			dtht2->loc[j] = (float)dtht->loc[j];
		for (int j = 0; j < 9; j++)
			dtht2->rot[j] = (float)dtht->rot[j];
		for (int k = 0; k < DTRACK_HAND_MAX_FINGER; k++)
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
		dtrack_marker_type *dtmt2 = &act_marker.at(i);
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
unsigned int DTrack::get_framecounter()
{
	return sdk->getFrameCounter();
}


/**
 * 	\brief	Get timestamp.
 *
 *	Refers to last received frame.
 *	@return	timestamp (-1 if information not available)
 */
double DTrack::get_timestamp()
{
	return sdk->getTimeStamp();
}


/**
 * 	\brief	Get number of standard bodies.
 *
 *	Refers to last received frame.
 *	@return	number of standard bodies
 */
int DTrack::get_num_body()
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
dtrack_body_type DTrack::get_body(int id)
{
	if (id < act_num_body)
	{
		return act_body[id];
	}
	dtrack_body_type dummy;
	memset(&dummy, 0, sizeof(dtrack_body_type));
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
int DTrack::get_num_flystick()
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
dtrack_flystick_type DTrack::get_flystick(int id)
{
	if (id < act_num_flystick)
	{
		return act_flystick[id];
	}
	dtrack_flystick_type dummy;
	memset(&dummy, 0, sizeof(dtrack_flystick_type));
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
int DTrack::get_num_meatool()
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
dtrack_meatool_type DTrack::get_meatool(int id)
{
	if (id < act_num_meatool)
	{
		return act_meatool[id];
	}
	dtrack_meatool_type dummy;
	memset(&dummy, 0, sizeof(dtrack_meatool_type));
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
int DTrack::get_num_hand()
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
dtrack_hand_type DTrack::get_hand(int id)
{
	if(id < act_num_hand){
		return act_hand[id];
	}
	dtrack_hand_type dummy;
	memset(&dummy, 0, sizeof(dtrack_hand_type));
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
int DTrack::get_num_marker()
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
dtrack_marker_type DTrack::get_marker(int index)
{
	if(index >= 0 && index < act_num_marker){
		return act_marker[index];
	}
	dtrack_marker_type dummy;
	memset(&dummy, 0, sizeof(dtrack_marker_type));
	dummy.id = 0;
	dummy.quality = -1;
	return dummy;
}


/**
 * 	\brief	Control cameras by remote commands to DTrack (UDP; ASCII protocol; Default off).
 *
 *	@param[in]	onoff	switch on/off
 *	@return		sending of remote commands was successfull
 */
bool DTrack::cmd_cameras(bool onoff)
{
	if (!valid())
		return false;
	remoteCameras = onoff;
	if (remoteCameras)
	{
		// switch cameras on
		if (remoteTracking)
		{
			sdk->sendCommand("dtrack 10 3");
			if (remoteSending)
			{
				return sdk->sendCommand("dtrack 31");
			}
		} else {
			sdk->sendCommand("dtrack 10 1");
		}
	} else {
		// switch cameras off
		if(remoteSending)
		{
			sdk->sendCommand("dtrack 32");
		}
		return sdk->sendCommand("dtrack 10 0");
	}
	return true;
}


/**
 * 	\brief	Control tracking calculation by remote commands to DTrack (UDP, ASCII protocol; Default: on).
 *
 *	@param[in]	onoff	Switch function (1 on, 0 off)
 *	@return 	sending of remote commands was successfull (1 yes, 0 no)
 */
bool DTrack::cmd_tracking(bool onoff)
{
	if (!valid())
		return false;
	remoteTracking = onoff;
	if (remoteCameras)
	{
		if (remoteTracking)
		{
			bool res = sdk->sendCommand("dtrack 10 3");
#ifdef OS_UNIX
			usleep(1200000);     // some delay (actually only necessary for older DTrack versions...)
#endif
#ifdef OS_WIN
			Sleep(1200);  // some delay (actually only necessary for older DTrack versions...)
#endif
			return res;
		} else {
			return sdk->sendCommand("dtrack 10 1");
		}
	}
	return true;
}

/**
 * 	\brief	Control sending of UDP output data by remote commands to DTrack (UDP, ASCII protocol; Default: on).
 *
 *	@param[in]	onoff	Switch function (1 on, 0 off)
 *	@return 	sending of remote commands was successfull (1 yes, 0 no)
 */
bool DTrack::cmd_sending_data(bool onoff)
{
	if (!valid())
		return false;
	remoteSending = onoff;
	if (!remoteCameras)
		return false;
	// cameras are on, so send command
	if (remoteSending)
	{
		return sdk->sendCommand("dtrack 31");
	}
	return sdk->sendCommand("dtrack 32");
}


/**
 * 	\brief	Control sending of a fixed number of UDP output data frames by remote commands to DTrack (UDP, ASCII protocol).
 *
 *	@param[in]	frames	Number of frames
 *	@return 	sending of remote commands was successfull (1 yes, 0 no)
 */
bool DTrack::cmd_sending_fixed_data(int frames)
{
	if (!valid())
		return false;
	if (remoteCameras)
	{
		// cameras are on, so send command
		std::stringstream cmd;
		cmd << "dtrack 33 " << frames;
		return sdk->sendCommand(cmd.str());
	}
	return true;
}
