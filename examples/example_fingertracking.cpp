/* DTrackSDK in C++: example_fingertracking.cpp
 *
 * C++ example using DTrackSDK FINGERTRACKING data.
 *
 * Copyright (c) 2013-2022 Advanced Realtime Tracking GmbH & Co. KG
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
 *  - example without DTrack2/DTRACK3 remote commands: just collects frames
 *  - output of various calculated FINGERTRACKING data
 *  - please start measurement manually e.g. in DTrack frontend application
 *  - for DTrackSDK v2.6.0 (or newer)
 */

#include "DTrackSDK.hpp"

#include <iostream>
#include <sstream>

#include "SampleMath.h"

// global DTrackSDK
static DTrackSDK* dt = NULL;

// sample finger structure with complete pose data of all joints and phalanxes (3 poses plus 1 position)
struct SampleFinger
{
	SampleLoc locRoot;    // position of root finger joint
	SampleRot rotRoot;    // rotation of inner phalanx

	SampleLoc locMiddle;  // position of middle finger joint
	SampleRot rotMiddle;  // rotation of middle phalanx

	SampleLoc locOuter;   // position of outer finger joint
	SampleRot rotOuter;   // rotation of outer phalanx

	SampleLoc locTip;     // position of finger tip
};

static void calculateSampleFinger( const DTrackHand* hand, int indFinger, SampleFinger& sampleFinger );

// prototypes
static void output_to_console();
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
			          << " " << SampleLoc( hand->loc ) << " " << SampleRot( hand->rot ) << std::endl;

			// calculating position for tip and poses for all joints of the fingers (in ART hand coordinate system):

			for ( int j = 0; j < hand->nfinger; j++ )
			{
				SampleFinger sampleFinger;
				calculateSampleFinger( hand, j, sampleFinger );

				std::cout << "   finger " << j << " tip " << sampleFinger.locTip << std::endl;
				std::cout << "            outer " << sampleFinger.locOuter << " " << sampleFinger.rotOuter << std::endl;
				std::cout << "            middle " << sampleFinger.locMiddle << " " << sampleFinger.rotMiddle << std::endl;
				std::cout << "            root " << sampleFinger.locRoot << " " << sampleFinger.rotRoot << std::endl;
			}
		}
		std::cout << std::endl;
	}
}


/**
 * \brief Calculate all finger's poses and positions wrt. ART hand coordinate system.
 */
static void calculateSampleFinger( const DTrackHand* hand, int indFinger, SampleFinger& sampleFinger )
{
	// finger tip:

	sampleFinger.locTip = SampleLoc( hand->finger[ indFinger ].loc );

	// outer finger phalanx:

	sampleFinger.rotOuter = SampleRot( hand->finger[ indFinger ].rot );

	SampleLoc phalanxOuter( -hand->finger[ indFinger ].lengthphalanx[ 0 ], 0.0, 0.0 );
	sampleFinger.locOuter = sampleFinger.rotOuter * phalanxOuter + sampleFinger.locTip;

	// middle finger phalanx:

	SampleRot jointOuter = SampleRot::rotationY( hand->finger[ indFinger ].anglephalanx[ 0 ] );
	sampleFinger.rotMiddle = sampleFinger.rotOuter * jointOuter;

	SampleLoc phalanxMiddle( -hand->finger[ indFinger ].lengthphalanx[ 1 ], 0.0, 0.0 );
	sampleFinger.locMiddle = sampleFinger.rotMiddle * phalanxMiddle + sampleFinger.locOuter;

	// inner finger phalanx:

	SampleRot jointInner = SampleRot::rotationY( hand->finger[ indFinger ].anglephalanx[ 1 ] );
	sampleFinger.rotRoot = sampleFinger.rotMiddle * jointInner;

	SampleLoc phalanxInner( -hand->finger[ indFinger ].lengthphalanx[ 2 ], 0.0, 0.0 );
	sampleFinger.locRoot = sampleFinger.rotRoot * phalanxInner + sampleFinger.locMiddle;
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

