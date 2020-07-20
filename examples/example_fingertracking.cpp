/* DTrackSDK: C++ example, A.R.T. GmbH
 *
 * C++ example using DTrackSDK FINGERTRACKING data
 *
 * Copyright (c) 2013-2020, Advanced Realtime Tracking GmbH
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
 * Version v2.6.0
 *
 * Purpose:
 *  - example without DTrack2/DTrack3 remote commands: just collects frames
 *  - output of various calculated FINGERTRACKING data
 *  - please start measurement manually e.g. in DTrack frontend application
 */

#include "DTrackSDK.hpp"

#include <iostream>
#include <sstream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// global DTrackSDK
static DTrackSDK* dt = NULL;

// prototypes
static void output_to_console();
static void trafo_loc2coo( double locres[ 3 ], const double loccoo[ 3 ], const double rotcoo[ 9 ], const double loc[ 3 ] );
static bool data_error_to_console();


/**
 * \brief Main.
 */
int main( int argc, char** argv )
{
	if ( argc != 2 )
	{
		std::cout << "Usage: example_fingertracking <data port>" << std::endl;
		return -1;
	}

	std::istringstream portstream( argv[ 1 ] );
	unsigned short port;
	portstream >> port;  // data port

	if ( portstream.fail() )
	{
		std::cout << "invalid port '" << argv[ 1 ] << "'" << std::endl;
		return -2;
	}

	// initialization:

	dt = new DTrackSDK( port );

	if ( ! dt->isDataInterfaceValid() )
	{
		std::cout << "DTrackSDK init error" << std::endl;
		return -3;
	}
	std::cout << "listening at local data port " << dt->getDataPort() << std::endl;

//	dt->setDataTimeoutUS( 3000000 );  // NOTE: change here timeout for receiving tracking data, if necessary
//	dt->setDataBufferSize( 100000 );  // NOTE: change here buffer size for receiving tracking data, if necessary

	// measurement:

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
		}
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
	          << " nhand " << dt->getNumHand() 
	          << std::endl;

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
				double lochand[ 3 ], locroom[ 3 ];

				// calculating the position for tip and all joints of the fingers (in hand and room coordinate system):

				for ( int k = 0; k < 3; k++ )
					lochand[ k ] = hand->finger[ j ].loc[ k ];  // finger tip (already in hand coordinate system)

				trafo_loc2coo( locroom, hand->loc, hand->rot, lochand );  // finger tip (in room coordinate system)

				std::cout << "   finger " << j
				          << " tip (hand) " << lochand[ 0 ] << " " << lochand[ 1 ] << " " << lochand[ 2 ]
				          << " tip (room) " << locroom[ 0 ] << " " << locroom[ 1 ] << " " << locroom[ 2 ]
				          << std::endl;

				lochand[ 0 ] = -hand->finger[ j ].lengthphalanx[ 0 ];  // first joint (in finger coordinate system)
				lochand[ 1 ] = lochand[ 2 ] = 0;

				trafo_loc2coo( lochand, hand->finger[ j ].loc, hand->finger[ j ].rot, lochand );  // first joint (in hand coordinate system)
				trafo_loc2coo( locroom, hand->loc, hand->rot, lochand );  // first joint (in room coordinate system)

				std::cout << "           "
				          << " joint 1 (hand) " << lochand[ 0 ] << " " << lochand[ 1 ] << " " << lochand[ 2 ]
				          << " joint 1 (room) " << locroom[ 0 ] << " " << locroom[ 1 ] << " " << locroom[ 2 ]
				          << std::endl;

				lochand[ 0 ] = -hand->finger[ j ].lengthphalanx[ 0 ]  // second joint (in finger coordinate system)
				             - hand->finger[ j ].lengthphalanx[ 1 ] * cos( hand->finger[ j ].anglephalanx[ 0 ] * M_PI / 180 );
				lochand[ 1 ] = 0;
				lochand[ 2 ] = hand->finger[ j ].lengthphalanx[ 1 ] * sin( hand->finger[ j ].anglephalanx[ 0 ] * M_PI / 180 );

				trafo_loc2coo( lochand, hand->finger[ j ].loc, hand->finger[ j ].rot, lochand );  // second joint (in hand coordinate system)
				trafo_loc2coo( locroom, hand->loc, hand->rot, lochand );  // second joint (in room coordinate system)

				std::cout << "           "
				          << " joint 2 (hand) " << lochand[ 0 ] << " " << lochand[ 1 ] << " " << lochand[ 2 ]
				          << " joint 2 (room) " << locroom[ 0 ] << " " << locroom[ 1 ] << " " << locroom[ 2 ]
				          << std::endl;

				lochand[ 0 ] = -hand->finger[ j ].lengthphalanx[ 0 ]  // third joint (in finger coordinate system)
				             - hand->finger[ j ].lengthphalanx[ 1 ] * cos( hand->finger[ j ].anglephalanx[ 0 ] * M_PI / 180 )
				             - hand->finger[ j ].lengthphalanx[ 2 ] * cos( ( hand->finger[ j ].anglephalanx[ 0 ] + hand->finger[ j ].anglephalanx[ 1 ] ) * M_PI / 180 );
				lochand[ 1 ] = 0;
				lochand[ 2 ] = hand->finger[ j ].lengthphalanx[ 1 ] * sin( hand->finger[ j ].anglephalanx[ 0 ] * M_PI / 180 )
				             + hand->finger[ j ].lengthphalanx[ 2 ] * sin( ( hand->finger[ j ].anglephalanx[ 0 ] + hand->finger[ j ].anglephalanx[ 1 ] ) * M_PI / 180 );

				trafo_loc2coo( lochand, hand->finger[ j ].loc, hand->finger[ j ].rot, lochand );  // third joint (in hand coordinate system)
				trafo_loc2coo( locroom, hand->loc, hand->rot, lochand );  // third joint (in room coordinate system)

				std::cout << "           "
				          << " joint 3 (hand) " << lochand[ 0 ] << " " << lochand[ 1 ] << " " << lochand[ 2 ]
				          << " joint 3 (room) " << locroom[ 0 ] << " " << locroom[ 1 ] << " " << locroom[ 2 ]
				          << std::endl;
			}
		}
		std::cout << std::endl;
	}
}


/**
 * \brief Transforms position into another coordinate system.
 */
static void trafo_loc2coo( double locres[ 3 ], const double loccoo[ 3 ], const double rotcoo[ 9 ], const double loc[ 3 ] )
{
	double tmploc[ 3 ];

	for ( int i = 0; i < 3; i++ )
		tmploc[ i ] = rotcoo[ i + 0 * 3 ] * loc[ 0 ] + rotcoo[ i + 1 * 3 ] * loc[ 1 ] + rotcoo[ i + 2 * 3 ] * loc[ 2 ];

	for ( int i = 0; i < 3; i++ )
		locres[ i ] = tmploc[ i ] + loccoo[ i ];
}


/**
 * \brief Prints error messages to console.
 *
 * @return No error occured?
 */
static bool data_error_to_console()
{
	if ( dt->getLastDataError() == DTrackSDK::ERR_TIMEOUT )
	{
		std::cout << "--- timeout while waiting for tracking data" << std::endl;
		return false;
	}

	if ( dt->getLastDataError() == DTrackSDK::ERR_NET )
	{
		std::cout << "--- error while receiving tracking data" << std::endl;
		return false;
	}

	if ( dt->getLastDataError() == DTrackSDK::ERR_PARSE )
	{
		std::cout << "--- error while parsing tracking data" << std::endl;
		return false;
	}

	return true;
}

