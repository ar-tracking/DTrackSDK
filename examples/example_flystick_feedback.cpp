/* DTrackSDK in C++: example_flystick_feedback.cpp
 *
 * C++ example to control a Flystick with feedback.
 *
 * Copyright (c) 2021-2022 Advanced Realtime Tracking GmbH & Co. KG
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
 *  - example with or without DTrack2/DTRACK3 remote commands
 *  - in communicating mode: starts measurement, collects some frames and stops measurement again
 *  - in listening mode: please start measurement manually e.g. in DTrack frontend application
 *  - for DTrackSDK v2.8.0 (or newer)
 */

#include "DTrackSDK.hpp"

#include <iostream>
#include <sstream>

// global DTrackSDK
static DTrackSDK* dt = NULL;

static bool sentFeedback = false;

// prototypes
static bool doFeedback( int flystickId );
static bool data_error_to_console();
static void messages_to_console();


/**
 * \brief Main.
 *
 * Control the Flystick feedback using the Flystick itself:
 * <li>Upper buttons start vibration pattern,
 * <li>Joystick button starts a beep with variable duration and frequency,
 * <li>Pressing the trigger button stops the program.
 */
int main( int argc, char** argv )
{
	if ( argc != 2 )
	{
		std::cout << "Usage: example_flystick_feedback [<server host/ip>:]<data port>" << std::endl;
		return -1;
	}

	// initialization:

	dt = new DTrackSDK( (const char *)argv[ 1 ] );

	if ( ! dt->isDataInterfaceValid() )
	{
		std::cout << "DTrackSDK init error" << std::endl;
		delete dt;
		return -3;
	}
	std::cout << "connected to ATC '" << argv[ 1 ] << "', listening at local data port " << dt->getDataPort() << std::endl;

//	dt->setCommandTimeoutUS( 30000000 );  // NOTE: change here timeout for exchanging commands, if necessary
//	dt->setDataTimeoutUS( 3000000 );      // NOTE: change here timeout for receiving tracking data, if necessary
//	dt->setDataBufferSize( 100000 );      // NOTE: change here buffer size for receiving tracking data, if necessary

	if ( dt->isCommandInterfaceValid() )  // ensure full access for DTrack2/DTRACK3 commands, if in communicating mode
	{
		if ( ! dt->isCommandInterfaceFullAccess() )
		{
			std::cout << "Full access to ATC required!" << std::endl;  // maybe DTrack2/3 frontend is still connected to ATC
			data_error_to_console();
			messages_to_console();
			delete dt;
			return -10;
		}
	}

	// measurement:

	if ( dt->isCommandInterfaceValid() )
	{
		if ( ! dt->startMeasurement() )  // start measurement
		{
			std::cout << "Measurement start failed!" << std::endl;
			data_error_to_console();
			messages_to_console();
			delete dt;
			return -4;
		}
	}

	int count = 0;
	bool isRunning = true;
	while ( isRunning )  // collect frames
	{
		count++;

		if ( dt->receive() )
		{
			int nfly = 0;
			for ( int id = 0; id < dt->getNumFlyStick(); id++ )
			{
				if ( ( dt->getFlyStick( id )->num_button >= 8 ) && ( dt->getFlyStick( id )->num_joystick >= 2 ) )
				{  // demo routine needs at least 8 buttons and 2 joystick values (e.g. Flystick2+)
					nfly++;

					if ( ! doFeedback( id ) )
						isRunning = false;
				}
			}

			if ( nfly == 0 )
			{
				std::cout << "No suitable Flystick identified!" << std::endl;
				isRunning = false;
			}
		}
		else
		{
			data_error_to_console();
			if ( dt->isCommandInterfaceValid() )  messages_to_console();
		}

		if ( ( count % 100 == 1 ) && dt->isCommandInterfaceValid() )
			messages_to_console();
	}

	if ( dt->isCommandInterfaceValid() )
	{
		dt->stopMeasurement();  // stop measurement
		messages_to_console();
	}

	delete dt;  // clean up
	return 0;
}


/**
 * \brief Process a frame and control Flystick feedback.
 *
 * @param[in] flystickId Id of Flystick
 * @return               Continue measurement?
 */
static bool doFeedback( int flystickId )
{
	const DTrackFlyStick* fly = dt->getFlyStick( flystickId );
	if ( fly == NULL )
	{
		std::cout << "DTrackSDK fatal error: invalid Flystick id " << flystickId << std::endl;
		return false;
	}

	if ( fly->button[ 0 ] != 0 )  // stop program if trigger button is pressed
		return false;

	// get beep feedback:

	if ( fly->button[ 5 ] != 0 )  // joystick button of Flystick2+
	{
		double beepDuration = 500.0 + fly->joystick[ 0 ] * 450.0;     // range 50 .. 950 ms
		double beepFrequency = 5000.0 + fly->joystick[ 1 ] * 3000.0;  // range 2000 .. 8000 Hz

		if ( ! sentFeedback )  // prevents permanent sending of feedback commands as long as button is pressed
			dt->flystickBeep( flystickId, beepDuration, beepFrequency );

		sentFeedback = true;
		return true;
	}

	// get vibration feedback:

	int vibrationPattern = 0;  // Flystick2+ supports up to 6 vibration pattern
	if ( fly->button[ 1 ] != 0 )  vibrationPattern = 1;
	if ( fly->button[ 2 ] != 0 )  vibrationPattern = 2;
	if ( fly->button[ 3 ] != 0 )  vibrationPattern = 3;
	if ( fly->button[ 4 ] != 0 )  vibrationPattern = 4;
	if ( fly->button[ 6 ] != 0 )  vibrationPattern = 5;  // button '5' (joystick button) is already used
	if ( fly->button[ 7 ] != 0 )  vibrationPattern = 6;

	if ( vibrationPattern > 0 )
	{
		if ( ! sentFeedback )  // prevents permanent sending of feedback commands as long as button is pressed
			dt->flystickVibration( flystickId, vibrationPattern );

		sentFeedback = true;
		return true;
	}

	sentFeedback = false;
	return true;
}


/**
 * \brief Prints error messages to console.
 *
 * @return No error occured?
 */
static bool data_error_to_console()
{
	bool ret = true;

	if ( dt->getLastDataError() != DTrackSDK::ERR_NONE )
	{
		if ( dt->getLastDataError() == DTrackSDK::ERR_TIMEOUT )
		{
			std::cout << "--- timeout while waiting for tracking data" << std::endl;
		}
		else if ( dt->getLastDataError() == DTrackSDK::ERR_NET )
		{
			std::cout << "--- error while receiving tracking data" << std::endl;
		}
		else if ( dt->getLastDataError() == DTrackSDK::ERR_PARSE )
		{
			std::cout << "--- error while parsing tracking data" << std::endl;
		}

		ret = false;
	}

	if ( dt->getLastServerError() != DTrackSDK::ERR_NONE )
	{
		if ( dt->getLastServerError() == DTrackSDK::ERR_TIMEOUT )
		{
			std::cout << "--- timeout while waiting for Controller command" << std::endl;
		}
		else if ( dt->getLastServerError() == DTrackSDK::ERR_NET )
		{
			std::cout << "--- error while receiving Controller command" << std::endl;
		}
		else if ( dt->getLastServerError() == DTrackSDK::ERR_PARSE )
		{
			std::cout << "--- error while parsing Controller command" << std::endl;
		}

		ret = false;
	}

	return ret;
}


/**
 * \brief Prints ATC messages to console.
 */
static void messages_to_console()
{
	while ( dt->getMessage() )
	{
		std::cout << "ATC message: \"" << dt->getMessageStatus() << "\" \"" << dt->getMessageMsg() << "\"" << std::endl;
	}
}

