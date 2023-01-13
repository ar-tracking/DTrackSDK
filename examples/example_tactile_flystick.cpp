/* DTrackSDK in C++: example_tactile_flystick.cpp
 *
 * C++ example using Flystick to control a tactile FINGERTRACKING device.
 *
 * Copyright (c) 2020-2023 Advanced Realtime Tracking GmbH & Co. KG
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
 *  - uses a Flystick to control an ART tactile FINGERTRACKING device
 *  - for DTrackSDK v2.8.0 (or newer)
 *
 * NOTE: link with 'winmm.lib' if compiling under Windows
 * NOTE: link with '-lrt' if compiling under older Linux (only for glibc versions before 2.17)
 */

#include <cmath>

#include "DTrackSDK.hpp"

#include <iostream>
#include <sstream>

// usually the following should work; otherwise define OS_* manually:
#if defined( _WIN32 ) || defined( WIN32 ) || defined( _WIN64 )
	#define OS_WIN   // for MS Windows
#else
	#define OS_UNIX  // for Unix (Linux)
#endif

#if defined( OS_WIN )
	#include <windows.h>
#endif
#if defined( OS_UNIX )
	#include <time.h>
#endif

// global DTrackSDK
static DTrackSDK* dt = NULL;

static const int NUMBER_OF_FINGERS = 3;  // for 3 fingers
static std::vector< double > strength( NUMBER_OF_FINGERS, 0.0 );

static const unsigned int REPEAT_PERIOD = 1000;  // period (in milliseconds) to repeat tactile command
static unsigned int lastTimeMs;

// prototypes
static bool doTactile( int flystickId, int handId );
static bool data_error_to_console();
static void messages_to_console();
static unsigned int getTimeMs();


/**
 * \brief Main.
 *
 * Control the tactile FINGERTRACKING device using the Flystick:
 * <li>Upper buttons set feedback for a finger with fixed strength,
 * <li>Joystick creates feedback for one or two fingers with variable strength,
 * <li>Pressing the trigger button stops the program.
 */
int main( int argc, char** argv )
{
	if ( argc != 4 )
	{
		std::cout << "Usage: example_tactile_flystick [<server host/ip>:]<data port> <Flystick id> <hand id>" << std::endl;
		return -1;
	}

	std::istringstream is( argv[ 2 ] );
	int flystickId;
	is >> flystickId;  // Flystick id, range 0 ..

	if ( is.fail() || ( flystickId < 0 ) )
	{
		std::cout << "invalid Flystick ID '" << argv[ 2 ] << "'" << std::endl;
		return -2;
	}

	is.clear();
	is.str( argv[ 3 ] );
	int handId;
	is >> handId;  // hand id, range 0 ..

	if ( is.fail() || ( handId < 0 ) )
	{
		std::cout << "invalid hand id '" << argv[ 3 ] << "'" << std::endl;
		return -2;
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

	lastTimeMs = getTimeMs();

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
	while ( true )  // collect frames
	{
		count++;

		if ( dt->receive() )
		{
			if ( flystickId >= dt->getNumFlyStick() || handId >= dt->getNumHand() )
			{
				std::cout << "Flystick ID or Hand ID doesn't exist!" << std::endl;
				break;
			}

			if ( ! doTactile( flystickId, handId ) )
				break;
		}
		else
		{
			data_error_to_console();
			if ( dt->isCommandInterfaceValid() )  messages_to_console();
		}

		if ( ( count % 100 == 1 ) && dt->isCommandInterfaceValid() )
			messages_to_console();
	}

	dt->tactileHandOff( handId, NUMBER_OF_FINGERS );

	if ( dt->isCommandInterfaceValid() )
	{
		dt->stopMeasurement();  // stop measurement
		messages_to_console();
	}

	delete dt;  // clean up
	return 0;
}


/**
 * \brief Process a frame and control tactile feedback device.
 *
 * @param[in] flystickId Id of Flystick
 * @param[in] handId     Id of ART tactile feedback device
 * @return               Continue measurement?
 */
static bool doTactile( int flystickId, int handId )
{
	const DTrackFlyStick* fly = dt->getFlyStick( flystickId );
	if ( fly == NULL )
	{
		std::cout << "DTrackSDK fatal error: invalid Flystick id " << flystickId << std::endl;
		return false;
	}

	if ( fly->button[ 0 ] != 0 )  // stop program if trigger button is pressed
		return false;

	// get new feedback strengths:

	std::vector< double > newStrength( NUMBER_OF_FINGERS, 0.0 );

	for ( int i = 0; i < NUMBER_OF_FINGERS; i++ )
	{
		if ( fly->button[ i + 1 ] != 0 )  // fixed strength if pressing upper buttons
			newStrength[ i ] = 0.5;
	}

	double joy = fly->joystick[ 0 ];
	if ( joy > 0.0 )  // variable strength if using joystick
	{
		newStrength[ 0 ] = joy;
	}
	else if ( joy < 0.0 )
	{
		newStrength[ 2 ] = -joy;
	}

	joy = fly->joystick[ 1 ];
	if ( joy > 0.0 )
	{
		newStrength[ 1 ] = joy;
	}

	// check if sending of tactile command is necessary:

	bool dosend = false;
	for ( int i = 0; i < NUMBER_OF_FINGERS; i++ )
	{
		if ( fabs( newStrength[ i ] - strength[ i ] ) >= 0.01 )
		{
			strength[ i ] = newStrength[ i ];
			dosend = true;
		}
	}

	unsigned int timeMs = getTimeMs();
	if ( timeMs - lastTimeMs >= REPEAT_PERIOD )  // repeat tactile command
		dosend = true;

	// send tactile command:

	if ( dosend )
	{
		dt->tactileHand( handId, strength );

		lastTimeMs = timeMs;
	}

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


/**
 * \brief Gets current time.
 *
 * @return system time (in milliseconds)
 */
static unsigned int getTimeMs()
{
#if defined( OS_WIN )
	return (unsigned int )timeGetTime();
#endif
#if defined( OS_UNIX )
	struct timespec ts;
	
	clock_gettime( CLOCK_MONOTONIC, &ts );

	return (unsigned int )ts.tv_sec * 1000u + (unsigned int )ts.tv_nsec / 1000000u;
#endif
}

