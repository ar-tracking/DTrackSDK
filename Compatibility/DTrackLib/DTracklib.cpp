/* DTrackSDK in C++: DTracklib.cpp
 *
 * Wrapper for deprecated SDK 'DTracklib' using DTrackSDK:
 * Functions to receive and process DTrack UDP packets (ASCII protocol).
 *
 * Copyright (c) 2000-2023 Advanced Realtime Tracking GmbH & Co. KG
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
 * Purpose:
 *   - receives DTrack UDP packets (ASCII protocol) and converts them into easier to handle data
 *   - sends DTrack remote commands (UDP)
 *   - DTrack network protocol due to: 'Technical Appendix DTrack v1.23 (April 6, 2005)'
 *   - for DTrack versions v1.16 - v1.23 (and compatible versions)
 *  - for DTrackSDK v2.5.0 (or newer)
 */

#include "DTracklib.hpp"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstring>

using namespace std;


/**
 * 	\brief	Constructor.
 *
 * @param[in]	udpport			UDP port number to receive data from DTrack
 * @param[in]	remote_ip		DTrack remote control: ip address of DTrack PC (NULL if not used)
 * @param[in]	remote_port 	port number of DTrack remote control (0 if not used)
 * @param[in]	udpbufsize		size of buffer for UDP packets (in bytes)
 * @param[in]	udptimeout_us	UDP timeout (receiving and sending) in us (micro sec)
 */
DTracklib::DTracklib(
        unsigned short udpport, char* remote_ip, unsigned short remote_port,
        int udpbufsize, unsigned long udptimeout_us
        )
{
	std::string s = "";
	if (remote_ip != NULL)
		s = remote_ip;

	sdk = new DTrackSDK( s, remote_port, udpport, DTrackSDK::SYS_DTRACK, udpbufsize, ( int )udptimeout_us, ( int )udptimeout_us );
	act_nbodycal = -1;
	act_nbody = act_nflystick = act_nmeatool = act_nmarker = act_nglove = 0;
}


/**
 * \brief Destructor.
 */
DTracklib::~DTracklib()
{
	delete sdk;
}


/**
 * 	\brief	Check if initialization was successfull.
 *
 *	@return Valid?
 */
bool DTracklib::valid()
{
	return sdk->isLocalDataPortValid();
}


/**
 * 	\brief Check last receive/send error (timeout)
 *
 *	@return	timeout occured?
 */
bool DTracklib::timeout()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_TIMEOUT);
}


/**
 * 	\brief Check last receive/send error (udp error)
 *
 * 	@return	udp error occured?
 */
bool DTracklib::udperror()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_NET);
}


/**
 * 	\brief Check last receive/send error (parser)
 *
 *	@return	error occured while parsing tracking data?
 */
bool DTracklib::parseerror()
{
	return (sdk->getLastDataError() == DTrackSDK::ERR_PARSE);
}


/**
 *	\brief	Receive and process one DTrack data packet (UDP; ASCII protocol)
 *
 *	@return	successful?
 */
bool DTracklib::receive()
{
	if (!sdk->receive())
		return false;
	// copy body data
	act_nbody = sdk->getNumBody();
	act_body.resize(act_nbody);
	for (int i = 0; i < act_nbody; i++)
	{
		const DTrack_Body_Type_d *dtbt = sdk->getBody(i);
		dtracklib_body_type *dtbt2 = &act_body.at(i);
		dtbt2->id = dtbt->id;
		dtbt2->quality = (float)dtbt->quality;
		for (int j = 0; j < 3; j++)
			dtbt2->loc[j] = (float)dtbt->loc[j];
		for (int j = 0; j < 3; j++)
			dtbt2->ang[j] = 0;
		for (int j = 0; j < 9; j++)
			dtbt2->rot[j] = (float)dtbt->rot[j];
	}
	// copy flystick data
	act_nflystick = sdk->getNumFlyStick();
	act_flystick.resize(act_nflystick);
	for (int i = 0; i < act_nflystick; i++)
	{
		const DTrack_FlyStick_Type_d *dtfst = sdk->getFlyStick(i);
		dtracklib_flystick_type *dtfst2 = &act_flystick.at(i);
		dtfst2->id = dtfst->id;
		dtfst2->quality = (float)dtfst->quality;
		dtfst2->bt = 0;
		for (int j = 0; j < min(dtfst->num_button, DTRACKLIB_FLYSTICK_MAX_BUTTON); j++)
		{
			if (dtfst->button[j] == 1)
				dtfst2->bt |= (dtfst->button[j] << j);
		}
		for (int j = 0; j < 3; j++)
			dtfst2->loc[j] = (float)dtfst->loc[j];
		for (int j = 0; j < 3; j++)
			dtfst2->ang[j] = 0;
		for (int j = 0; j < 9; j++)
			dtfst2->rot[j] = (float)dtfst->rot[j];
	}
	// copy measurement tool data
	act_nmeatool = sdk->getNumMeaTool();
	act_meatool.resize(act_nmeatool);
	for (int i = 0; i < act_nmeatool; i++)
	{
		const DTrack_MeaTool_Type_d *dtmtt = sdk->getMeaTool(i);
		dtracklib_meatool_type *dtmtt2 = &act_meatool.at(i);
		dtmtt2->id = dtmtt->id;
		dtmtt2->quality = (float)dtmtt->quality;
		dtmtt2->bt = 0;
		for (int j = 0; j < min(dtmtt->num_button, DTRACKLIB_FLYSTICK_MAX_BUTTON); j++)
		{
			if (dtmtt->button[j] == 1)
				dtmtt2->bt |= (dtmtt->button[j] << j);
		}
		for (int j = 0; j < 3; j++)
			dtmtt2->loc[j] = (float)dtmtt->loc[j];
		for (int j = 0; j < 9; j++)
			dtmtt2->rot[j] = (float)dtmtt->rot[j];
	}
	// copy hand data
	act_nglove = sdk->getNumHand();
	act_glove.resize(act_nglove);
	for (int i = 0; i < act_nglove; i++)
	{
		const DTrack_Hand_Type_d *dtht = sdk->getHand(i);
		dtracklib_glove_type *dtht2 = &act_glove.at(i);
		dtht2->id = dtht->id;
		dtht2->quality = (float)dtht->quality;
		dtht2->lr = dtht->lr;
		dtht2->nfinger = dtht->nfinger;
		for (int j = 0; j < 3; j++)
			dtht2->loc[j] = (float)dtht->loc[j];
		for (int j = 0; j < 9; j++)
			dtht2->rot[j] = (float)dtht->rot[j];
		for (int k = 0; k < DTRACKLIB_HAND_MAX_FINGER; k++)
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
	act_nmarker = sdk->getNumMarker();
	act_marker.resize(act_nmarker);
	for (int i = 0; i < act_nmarker; i++)
	{
		const DTrack_Marker_Type_d *dtmt = sdk->getMarker(i);
		dtracklib_marker_type *dtmt2 = &act_marker.at(i);
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
unsigned long DTracklib::get_framenr()
{
	return (unsigned long)sdk->getFrameCounter();
}


/**
 * 	\brief	Get timestamp.
 *
 *	Refers to last received frame.
 *	@return	timestamp (-1 if information not available)
 */
double DTracklib::get_timestamp()
{
	return sdk->getTimeStamp();
}


/**
 * 	\brief	Get number of calibrated bodies.
 *
 *	@return	number of calibrated bodies; -1, if information not available
 */
int DTracklib::get_nbodycal()
{
	return act_nbodycal;
}


/**
 * 	\brief	Get number of standard bodies.
 *
 *	Refers to last received frame.
 *	@return	number of standard bodies
 */
int DTracklib::get_nbody()
{
	return act_nbody;
}


/**
 * 	\brief	Get 6d data.
 *
 *	Refers to last received frame.
 *	@param[in]	id	id, range 0 .. nbody - 1
 *	@return		id-th standard body data.
 */
dtracklib_body_type DTracklib::get_body(int id)
{
	if ((id < 0)||(id >= act_nbody))
	{
		dtracklib_body_type dummy;
		memset(&dummy, 0, sizeof(dtracklib_body_type));
		return dummy;
	}
	return act_body[id];
}


/**
 * 	\brief	Get number of flysticks.
 *
 *	Refers to last received frame.
 *	@return	number of flysticks
 */
int DTracklib::get_nflystick()
{
	return act_nflystick;
}


/**
 * 	\brief	Get 6df data.
 *
 *	Refers to last received frame.
 *	@param[in]	id	id, range 0 .. nflystick - 1
 *	@return		id-th flystick data
 */
dtracklib_flystick_type DTracklib::get_flystick(int id)
{
	if ((id < 0)||(id >= act_nflystick))
	{
		dtracklib_flystick_type dummy;
		memset(&dummy, 0, sizeof(dtracklib_flystick_type));
		return dummy;
	}
	return act_flystick[id];
}


/**
 * 	\brief	Get number of measurement tools.
 *
 *	Refers to last received frame.
 *	@return	number of measurement tools
 */
int DTracklib::get_nmeatool()
{
	return act_nmeatool;
}


/**
 * 	\brief	Get 6dmt data.
 *
 *	Refers to last received frame.
 *	@param[in]	id	id, range 0 .. nmeatool - 1
 *	@return		id-th measurement tool data
 */
dtracklib_meatool_type DTracklib::get_meatool(int id)
{
	if(id < 0 || id >= act_nmeatool){
		dtracklib_meatool_type dummy;
		memset(&dummy, 0, sizeof(dtracklib_meatool_type));
		return dummy;
	}
	
	return act_meatool[id];
}


/**
 * 	\brief	Get number of fingertracking hands.
 *
 *	Refers to last received frame.
 *	@return	number of fingertracking hands
 */
int DTracklib::get_nglove()
{
	return act_nglove;
}


/**
 * 	\brief	Get gl data.
 *
 *	Refers to last received frame.
 *	@param[in]	id	id, range 0 .. nglove - 1
 *	@return		id-th Fingertracking hand data.
 */
dtracklib_glove_type DTracklib::get_glove(int id)
{
	if(id < 0 || id >= act_nglove){
		dtracklib_glove_type dummy;
		memset(&dummy, 0, sizeof(dtracklib_glove_type));
		return dummy;
	}
	
	return act_glove[id];
}


/**
 * 	\brief	Get number of single markers
 *
 *	Refers to last received frame.
 *	@return	number of single markers
 */
int DTracklib::get_nmarker()
{
	return act_nmarker;
}


/**
 * 	\brief	Get 3d data.
 *
 *	Refers to last received frame.
 *	@param[in]	index	index, range 0 .. nmarker - 1
 *	@return		i-th single marker data
 */
dtracklib_marker_type DTracklib::get_marker(int index)
{
	if(index < 0 || index >= act_nmarker){
		dtracklib_marker_type dummy;
		memset(&dummy, 0, sizeof(dtracklib_marker_type));
		return dummy;
	}
	return act_marker[index];
}


/**
 *	\brief	Send one remote control command (UDP; ASCII protocol) to DTrack.
 *
 *	@param[in]	cmd		command code
 *	@param[in]	val		additional value (depends on command code), default 0
 *
 *	@return sending was successful
 */
bool DTracklib::send(unsigned short cmd, int val)
{
	if (!sdk->isLocalDataPortValid())
		return false;
	std::stringstream c;
	c << "dtrack ";
	switch(cmd){
		case DTRACKLIB_CMD_CAMERAS_OFF:
			c << "10 0";
			break;
		case DTRACKLIB_CMD_CAMERAS_ON:
			c << "10 1";
			break;
		case DTRACKLIB_CMD_CAMERAS_AND_CALC_ON:
			c << "10 3";
			break;
		case DTRACKLIB_CMD_SEND_DATA:
			c << "31";
			break;
		case DTRACKLIB_CMD_STOP_DATA:
			c << "32";
			break;
		case DTRACKLIB_CMD_SEND_N_DATA:
			c << "33 " << val;
			break;
		default:
			return 0;
	}
	sdk->sendCommand(c.str());
	return true;
}
