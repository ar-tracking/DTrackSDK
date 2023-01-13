/* DTrackSDK in C++: SampleMath.h
 *
 * Minimum math classes; to be replaced by your favourite library.
 *
 * Copyright (c) 2022 Advanced Realtime Tracking GmbH & Co. KG
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
 */

#include <iostream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class SampleRot;

/**
 * \brief Minimum class for a position.
 */
class SampleLoc
{
public:

	SampleLoc()
	{}

	SampleLoc( const double loc[ 3 ] )
	{
		for ( int i = 0; i < 3; i++ )
			m_loc[ i ] = loc[ i ];
	}

	SampleLoc( double loc0, double loc1, double loc2 )
	{
		m_loc[ 0 ] = loc0;
		m_loc[ 1 ] = loc1;
		m_loc[ 2 ] = loc2;
	}

	double operator[]( int ind ) const
	{
		return m_loc[ ind ];
	}

	SampleLoc operator+( const SampleLoc& other ) const
	{
		SampleLoc tmploc;

		for ( int i = 0; i < 3; i++ )
			tmploc.m_loc[ i ] = m_loc[ i ] + other.m_loc[ i ];

		return tmploc;
	}

	friend SampleRot;

private:

	double m_loc[ 3 ];
};


/**
 * \brief Minimum class for a rotation.
 */
class SampleRot
{
public:

	SampleRot()
	{}

	SampleRot( const double rot[ 9 ] )
	{
		for ( int i = 0; i < 9; i++ )
			m_rot[ i ] = rot[ i ];
	}

	double operator[]( int ind ) const
	{
		return m_rot[ ind ];
	}

	SampleRot operator*( const SampleRot& other ) const
	{
		SampleRot tmprot;

		for ( int i = 0; i < 3; i++ )
		{
			for ( int j = 0; j < 3; j++ )
			{
				double t = 0.0;
				for ( int k = 0; k < 3; k++ )
					t += m_rot[ i + k * 3 ] * other.m_rot[ k + j * 3 ];

				tmprot.m_rot[ i + j * 3 ] = t;
			}
		}

		return tmprot;
	}

	SampleLoc operator*( const SampleLoc& other ) const
	{
		SampleLoc tmploc;

		for ( int i = 0; i < 3; i++ )
		{
			double t = 0.0;
			for ( int k = 0; k < 3; k++ )
				t += m_rot[ i + k * 3 ] * other.m_loc[ k ];

			tmploc.m_loc[ i ] = t;
		}

		return tmploc;
	}

	static SampleRot rotationY( double ang )  // rotation around Y-axis
	{
		SampleRot tmprot;

		double cosang = cos( ang * M_PI / 180 );
		double sinang = sin( ang * M_PI / 180 );

		tmprot.m_rot[ 0 + 0 * 3 ] = tmprot.m_rot[ 2 + 2 * 3 ] = cosang;
		tmprot.m_rot[ 2 + 0 * 3 ] = -sinang;
		tmprot.m_rot[ 0 + 2 * 3 ] = sinang;
		tmprot.m_rot[ 1 + 1 * 3 ] = 1.0;
		tmprot.m_rot[ 0 + 1 * 3 ] = tmprot.m_rot[ 1 + 0 * 3 ] = tmprot.m_rot[ 2 + 1 * 3 ] = tmprot.m_rot[ 1 + 2 * 3 ] = 0.0;

		return tmprot;
	}

private:

	double m_rot[ 9 ];
};


/**
 * \brief Output of a position to console.
 */
std::ostream& operator<<( std::ostream& os, const SampleLoc& loc )
{
	os << "loc " << loc[ 0 ] << " " << loc[ 1 ] << " " << loc[ 2 ];

	return os;
}

/**
 * \brief Output of a rotation to console.
 */
std::ostream& operator<<( std::ostream& os, const SampleRot& rot )
{
	os << "rot " << rot[ 0 ] << " " << rot[ 1 ] << " " << rot[ 2 ] << " " << rot[ 3 ] << " " << rot[ 4 ] << " "
	             << rot[ 5 ] << " " << rot[ 6 ] << " " << rot[ 7 ] << " " << rot[ 8 ];

	return os;
}

