/* DTrackSDK in C++: example_universal.cpp
 *
 * C++ example using universal DTrackSDK constructor for all modes.
 *
 * Copyright (c) 2019-2023 Advanced Realtime Tracking GmbH & Co. KG
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

// global DTrackSDK
static DTrackSDK* dt = NULL;

// prototypes
static void output_to_console();
static bool data_error_to_console();
static void messages_to_console();


/**
 * \brief Main.
 */
int main( int argc, char** argv )
{
	if ( argc != 2 )
	{
		std::cout << "Usage: example_universal [<server host/ip>:]<data port>" << std::endl;
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
	while ( count++ < 1000 )  // collect 1000 frames
	{
		if ( dt->receive() )
		{
			output_to_console();
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
 * \brief Prints current tracking data to console.
 */
static void output_to_console()
{
	std::cout.precision( 3 );
	std::cout.setf( std::ios::fixed, std::ios::floatfield );

	std::cout << std::endl << "frame " << dt->getFrameCounter() << " ts " << dt->getTimeStamp()
	          << " nbod " << dt->getNumBody() << " nfly " << dt->getNumFlyStick()
	          << " nmea " << dt->getNumMeaTool() << " nmearef " << dt->getNumMeaRef() 
	          << " nhand " << dt->getNumHand() << " nmar " << dt->getNumMarker() 
	          << " nhuman " << dt->getNumHuman() << " ninertial " << dt->getNumInertial()
	          << " status " << ( dt->isStatusAvailable() ? "yes" : "no" )
	          << std::endl;

	// Standard bodies:
	for ( int i = 0; i < dt->getNumBody(); i++ )
	{
		const DTrackBody* body = dt->getBody( i );
		if ( body == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid body id " << i << std::endl;
			break;
		}

		if ( ! body->isTracked() )
		{
			std::cout << "bod " << body->id << " not tracked" << std::endl;
		}
		else
		{
			std::cout << "bod " << body->id << " qu " << body->quality
			          << " loc " << body->loc[ 0 ] << " " << body->loc[ 1 ] << " " << body->loc[ 2 ]
			          << " rot " << body->rot[ 0 ] << " " << body->rot[ 1 ] << " " << body->rot[ 2 ]
			          << " "     << body->rot[ 3 ] << " " << body->rot[ 4 ] << " " << body->rot[ 5 ]
			          << " "     << body->rot[ 6 ] << " " << body->rot[ 7 ] << " " << body->rot[ 8 ]
			          << std::endl;

			DTrackQuaternion quat = body->getQuaternion();
			std::cout << "bod " << body->id << " quatw " << quat.w
			          << " quatxyz " << quat.x << " " << quat.y << " " << quat.z << std::endl;
		}
	}

	// A.R.T. Flysticks:
	for ( int i = 0; i < dt->getNumFlyStick(); i++ )
	{
		const DTrackFlyStick* flystick = dt->getFlyStick( i );
		if ( flystick == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid Flystick id " << i << std::endl;
			break;
		}

		if ( ! flystick->isTracked() )
		{
			std::cout << "fly " << flystick->id << " not tracked" << std::endl;
		}
		else
		{
			std::cout << "flystick " << flystick->id << " qu " << flystick->quality
			          << " loc " << flystick->loc[ 0 ] << " " << flystick->loc[ 1 ] << " " << flystick->loc[ 2 ]
			          << " rot " << flystick->rot[ 0 ] << " " << flystick->rot[ 1 ] << " " << flystick->rot[ 2 ]
			          << " "     << flystick->rot[ 3 ] << " " << flystick->rot[ 4 ] << " " << flystick->rot[ 5 ]
			          << " "     << flystick->rot[ 6 ] << " " << flystick->rot[ 7 ] << " " << flystick->rot[ 8 ]
			          << std::endl;
		}

		std::cout << "      btn";
		for ( int j = 0; j < flystick->num_button; j++ )
		{
			std::cout << " " << flystick->button[ j ];
		}
		std::cout << " joy";
		for ( int j = 0; j < flystick->num_joystick; j++ )
		{
			std::cout << " " << flystick->joystick[ j ];
		}
		std::cout << std::endl;
	}

	// Measurement tools:
	for ( int i = 0; i < dt->getNumMeaTool(); i++ )
	{
		const DTrackMeaTool* meatool = dt->getMeaTool( i );
		if ( meatool == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid Measurement tool id " << i << std::endl;
			break;
		}

		if ( ! meatool->isTracked() )
		{
			std::cout << "mea " << meatool->id << " not tracked" << std::endl;
		}
		else
		{
			std::cout << "mea " << meatool->id << " qu " << meatool->quality
			          << " loc " << meatool->loc[ 0 ] << " " << meatool->loc[ 1 ] << " " << meatool->loc[ 2 ]
			          << " rot " << meatool->rot[ 0 ] << " " << meatool->rot[ 1 ] << " " << meatool->rot[ 2 ]
			          << " "     << meatool->rot[ 3 ] << " " << meatool->rot[ 4 ] << " " << meatool->rot[ 5 ]
			          << " "     << meatool->rot[ 6 ] << " " << meatool->rot[ 7 ] << " " << meatool->rot[ 8 ]
			          << std::endl;
		}

		if ( meatool->tipradius > 0.0 )
		{
			std::cout << "      radius " << meatool->tipradius << std::endl;
		}

		if ( meatool->num_button > 0 )
		{
			std::cout << "      btn";
			for ( int j = 0; j < meatool->num_button; j++ )
			{
				std::cout << " " << meatool->button[ j ];
			}
			std::cout << std::endl;
		}
	}

	// Measurement references:
	for ( int i = 0; i < dt->getNumMeaRef(); i++ )
	{
		const DTrackMeaRef* mearef = dt->getMeaRef( i );
		if ( mearef == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid Measurement reference id " << i << std::endl;
			break;
		}

		if ( ! mearef->isTracked() )
		{
			std::cout << "mearef " << mearef->id << " not tracked" << std::endl;
		}
		else
		{
			std::cout << "mearef " << mearef->id << " qu " << mearef->quality
			          << " loc " << mearef->loc[ 0 ] << " " << mearef->loc[ 1 ] << " " << mearef->loc[ 2 ]
			          << " rot " << mearef->rot[ 0 ] << " " << mearef->rot[ 1 ] << " " << mearef->rot[ 2 ]
			          << " "     << mearef->rot[ 3 ] << " " << mearef->rot[ 4 ] << " " << mearef->rot[ 5 ]
			          << " "     << mearef->rot[ 6 ] << " " << mearef->rot[ 7 ] << " " << mearef->rot[ 8 ]
			          << std::endl;
		}
	}

	// Single markers:
	for ( int i = 0; i < dt->getNumMarker(); i++ )
	{
		const DTrackMarker* marker = dt->getMarker( i );
		if ( marker == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid marker index " << i << std::endl;
			break;
		}

		std::cout << "mar " << marker->id << " qu " << marker->quality
		          << " loc " << marker->loc[ 0 ] << " " << marker->loc[ 1 ] << " " << marker->loc[ 2 ]
		          << std::endl;
	}

	// A.R.T. FINGERTRACKING hands:
	for ( int i = 0; i < dt->getNumHand(); i++ )
	{
		const DTrackHand* hand = dt->getHand( i );
		if ( hand == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid FINGERTRACKING id " << i << std::endl;
			break;
		}

		if ( ! hand->isTracked() )
		{
			std::cout << "hand " << hand->id << " not tracked" << std::endl;
		}
		else
		{
			std::cout << "hand " << hand->id << " qu " << hand->quality
			          << " lr " << ( ( hand->lr == 0 ) ? "left" : "right") << " nf " << hand->nfinger
			          << " loc " << hand->loc[ 0 ] << " " << hand->loc[ 1 ] << " " << hand->loc[ 2 ]
			          << " rot " << hand->rot[ 0 ] << " " << hand->rot[ 1 ] << " " << hand->rot[ 2 ]
			          << " "     << hand->rot[ 3 ] << " " << hand->rot[ 4 ] << " " << hand->rot[ 5 ]
			          << " "     << hand->rot[ 6 ] << " " << hand->rot[ 7 ] << " " << hand->rot[ 8 ]
			          << std::endl;

			for ( int j = 0; j < hand->nfinger; j++ )
			{
				std::cout << "       fi " << j
				          << " loc " << hand->finger[ j ].loc[ 0 ] << " " << hand->finger[ j ].loc[ 1 ] << " " << hand->finger[ j ].loc[ 2 ]
				          << " rot " << hand->finger[ j ].rot[ 0 ] << " " << hand->finger[ j ].rot[ 1 ] << " " << hand->finger[ j ].rot[ 2 ]
				          << " "     << hand->finger[ j ].rot[ 3 ] << " " << hand->finger[ j ].rot[ 4 ] << " " << hand->finger[ j ].rot[ 5 ]
				          << " "     << hand->finger[ j ].rot[ 6 ] << " " << hand->finger[ j ].rot[ 7 ] << " " << hand->finger[ j ].rot[ 8 ]
				          << std::endl;
				std::cout << "       fi " << j
				          << " tip " << hand->finger[ j ].radiustip
				          << " pha " << hand->finger[ j ].lengthphalanx[ 0 ] << " " << hand->finger[ j ].lengthphalanx[ 1 ]
				          << " "     << hand->finger[ j ].lengthphalanx[ 2 ]
				          << " ang " << hand->finger[ j ].anglephalanx[ 0 ] << " " << hand->finger[ j ].anglephalanx[ 1 ]
				          << std::endl;
			}
		}
	}

	// A.R.T human models:
	if ( dt->getNumHuman() < 1 )
	{
		std::cout << "no human model data" << std::endl;
	}

	for ( int i = 0; i < dt->getNumHuman(); i++ )
	{
		const DTrackHuman* human = dt->getHuman( i );
		if ( human == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid human model id " << i << std::endl;
			break;
		}

		if ( ! human->isTracked() )
		{
			std::cout << "human " << human->id << " not tracked" << std::endl;
		}
		else
		{
			std::cout << "human " << human->id << " num joints " << human->num_joints << std::endl;
			for ( int j = 0; j < human->num_joints; j++ )
			{
				if ( ! human->joint[ j ].isTracked() )
				{
					std::cout << "joint " << human->joint[ j ].id << " not tracked" << std::endl;
				}
				else
				{
					std::cout << "joint " << human->joint[ j ].id << " qu " << human->joint[j].quality
					          << " loc " << human->joint[ j ].loc[ 0 ] << " " << human->joint[j].loc[ 1 ] << " " << human->joint[ j ].loc[ 2 ]
					          << " rot " << human->joint[ j ].rot[ 0 ] << " " << human->joint[j].rot[ 1 ] << " " << human->joint[ j ].rot[ 2 ]
					          << " "     << human->joint[ j ].rot[ 3 ] << " " << human->joint[j].rot[ 4 ] << " " << human->joint[ j ].rot[ 5 ]
					          << " "     << human->joint[ j ].rot[ 6 ] << " " << human->joint[j].rot[ 7 ] << " " << human->joint[ j ].rot[ 8 ]
					          << std::endl;
				}
			}
		}
		std::cout << std::endl;
	}

	// Hybrid bodies:
	if ( dt->getNumInertial() < 1 )
	{
		std::cout << "no inertial body data" << std::endl;
	}

	for ( int i = 0; i < dt->getNumInertial(); i++ )
	{
		const DTrackInertial* inertial = dt->getInertial( i );
		if ( inertial == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid hybrid body id " << i << std::endl;
			break;
		}

		std::cout << " inertial body " << inertial->id << " st " << inertial->st << " error " << inertial->error << std::endl;
		if ( inertial->isTracked() )
		{
			std::cout << " loc " << inertial->loc[ 0 ] << " " << inertial->loc[ 1 ] << " " << inertial->loc[ 2 ]
			          << " rot " << inertial->rot[ 0 ] << " " << inertial->rot[ 1 ] << " " << inertial->rot[ 2 ]
			          << " "     << inertial->rot[ 3 ] << " " << inertial->rot[ 4 ] << " " << inertial->rot[ 5 ]
			          << " "     << inertial->rot[ 6 ] << " " << inertial->rot[ 7 ] << " " << inertial->rot[ 8 ]
			          << std::endl;
		}
	}

	// System status:
	if ( ! dt->isStatusAvailable() )
	{
		std::cout << "no system status data" << std::endl;
	}
	else
	{
		const DTrackStatus* status = dt->getStatus();
		if ( status == NULL )
		{
			std::cout << "DTrackSDK fatal error: invalid system status" << std::endl;
		}
		else
		{
			// general status values
			std::cout << "status gen nc " << status->numCameras
			          << " nb " << status->numTrackedBodies << " nm " << status->numTrackedMarkers << std::endl;

			// message statistics
			std::cout << "status msg nce " << status->numCameraErrorMessages << " ncw " << status->numCameraWarningMessages
			          << " noe " << status->numOtherErrorMessages << " now " << status->numOtherWarningMessages
			          << " ni " << status->numInfoMessages << std::endl;

			// camera status values
			for ( int i = 0; i < status->numCameras; i++ )
			{
				std::cout << "status cam " << status->cameraStatus[ i ].idCamera
				          << " ns " << status->cameraStatus[ i ].numReflections
				          << " nu " << status->cameraStatus[ i ].numReflectionsUsed
				          << " mi " << status->cameraStatus[ i ].maxIntensity << std::endl;
			}
		}
	}
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

