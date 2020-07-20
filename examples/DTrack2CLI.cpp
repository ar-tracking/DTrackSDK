/* DTrack2CLI (C++), A.R.T. GmbH
 *
 * DTrack2CLI:
 *    C++ based Command Line Interface for DTrack2 or DTrack3
 *
 * Copyright (c) 2016-2020, Advanced Realtime Tracking GmbH
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
 *  - Command Line Interface for DTrack2 or DTrack3:
 *	   processes DTrack2/3 commands from command line or read from files
 *
 */

#include "DTrackSDK.hpp"

#include <iostream>
#include <fstream>

// global DTrackSDK
static DTrackSDK* dt = NULL;

// version of DTrack2CLI
static const unsigned int VERSION_MAJOR = 1;
static const unsigned int VERSION_MINOR = 0;
static const unsigned int VERSION_PATCH = 0;

enum ERRORS {
	ERR_WRONG_INPUT_PARAMETER = -101 ,		//!< wrong (number of) input parameter(s) 
	ERR_WRONG_USAGE = -102 ,				//!< wrong usage of DTrack2CLI
	ERR_DTRACKSDK_INIT = -103 ,				//!< DTrackSDK Init Error (no TCP connection to DTrack2)
	ERR_DTRACK2_CMD_SPELLING = -104 ,		//!< wrong spelling of a DTrack2 command
	ERR_OPEN_FILE = -105 ,					//!< unable to open file
	ERR_UNKNOWN = -106						//!< unknown error occured
};

/**
 *	\brief Gets all event messages from backend and prints these to cerr
 */
static void dtrack2_get_and_print_all_event_messages()
{
	while ( dt->getMessage() )
	{
		std::cerr << dt->getMessageOrigin()
			      << " " << dt->getMessageStatus()
			      << " " << dt->getMessageFrameNr()
			      << " 0x" << std::hex << dt->getMessageErrorId() << std::dec
			      << " " << dt->getMessageMsg() 
			      << std::endl;
	}
}


/**
*	\brief Checks if DTrack2/3 backend returned an error and prints it
*	
*	Checks for error received from DTrack2/3 and prints it,
*	makes sure DTrack2CLI displays the same error number as DTrack2/3 backend.
*	Also prints all event messages if an error appeared.
*/
static int dtrack2_error_to_console()
{
	int dtrack2Error = dt->getLastDTrackError();
	if ( dtrack2Error )
	{
		std::string errorMessage = dt->getLastDTrackErrorDescription();
		std::cerr << "error " << dtrack2Error << ": " << errorMessage << std::endl;
		dtrack2_get_and_print_all_event_messages();
	}
	return dtrack2Error;
}


/**
*	\brief Show usage/help
*/
static void show_help( const std::string& programName )
{
	std::cout << "DTrack2CLI v" << VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << std::endl;
	std::cout << "Usage: " << programName << " <ATC hostname or ip> [<action> ...]" << std::endl;
	std::cout << "Apply an action to the ART Controller (ATC) specified by ACTION(s)" << std::endl;
	std::cout << "with <ATC hostname or ip> being either the IP address or the" << std::endl;
	std::cout << "hostname of the ART Controller e.g.: atc-123456 or 12.34.56.78" << std::endl;
	std::cout << "Available actions:" << std::endl;
	std::cout << "  -meastart                   start measurement" << std::endl;
	std::cout << "  -meastop                    stop measurement" << std::endl;
	std::cout << "  -shutdown                   shut down the ART Controller" << std::endl;
	std::cout << "  -get <parameter>            read and display the value of a DTrack2/3 parameter" << std::endl;
	std::cout << "  -set <parameter> <value>    change the value of a DTrack2/3 parameter" << std::endl;
	std::cout << "  -cmd <dtrack2 command>      send DTrack2 command directly" << std::endl;
	std::cout << "  -f <filename>               read and execute DTrack2/3 commands from a file" << std::endl;
	std::cout << "  -h, --help, /?              display this help" << std::endl;
}


/**
*	\brief Checks if all command line arguments are correct
*/
static int checkInput( int argc, char** argv )
{
	// cursor (points to current processed argument)
	int iArgument = 2;

	bool errorOccured = false;
	
	// checks each single argument for correctness
	while ( ( argc >= iArgument ) )
	{
		// only occurs the first time this loop is executed
		if ( iArgument == 2 )
			++iArgument;

		// takes one argument from input
		std::string command( argv [ iArgument - 1 ] );

		// check for different possible commands
		// command with 0 arguments
		if ( ( command.compare( "-h" ) == 0 ) ||
			 ( command.compare( "--help" ) == 0 ) ||
			 ( command.compare( "/?" ) == 0 ) ||
			 ( command.compare( "-meastart" ) == 0 ) ||
			 ( command.compare( "-meastop" ) == 0 ) ||
			 ( command.compare( "-shutdown" ) == 0 ) )
		{
			++iArgument;
		}
		// command with 1 argument
		else if ( ( command.compare( "-get" ) == 0 ) || 
			      ( command.compare( "-f" ) == 0 ) ||
			      ( command.compare( "-cmd" ) == 0 ) )
		{
			bool haveEnoughCommands = ( argc > iArgument );
			if ( haveEnoughCommands )
				iArgument += 2;
			else
				errorOccured = true;
		}
		// command with 2 arguments
		else if ( ( command.compare( "-set" ) == 0 ) )
		{
			bool haveEnoughCommands = ( argc > iArgument + 1 );
			if ( haveEnoughCommands )
				iArgument += 3;
			else
				errorOccured = true;
		}
		// unknown command
		else
			errorOccured = true;

		if ( errorOccured )
		{
			std::cerr << "Please check input parameters! (See help)" << std::endl;
			//show_help( argv[ 0 ] ); // don't show here (clarified with KA, 21.10.2016)
			return ERR_WRONG_INPUT_PARAMETER;
		}
	}
	return 0;
}


/**
*	\brief If no measurement is running, starts measurement
*/
static int start_measurement()
{
	std::string trackingStatus;

	// check tracking status
	bool success = dt->getParam( "status active", trackingStatus );
	if ( success )
	{
		// check the answer (possibilities: none, cal, mea, wait or err)
		if ( trackingStatus.compare( "mea" ) != 0 && trackingStatus.compare( "wait" ) != 0 )
		{
			if ( !dt->startMeasurement() )
			{
				int errorOccured = dtrack2_error_to_console();
				if ( errorOccured )
					return errorOccured;
			}
		}
	}
	else
	{
		int errorOccured = dtrack2_error_to_console();
		if ( errorOccured )
			return errorOccured;
	}
	return 0;
}


/**
*	\brief If measurement is running, stops measurement
*/
static int stop_measurement()
{
	std::string trackingStatus;

	// check if measurement is already running
	bool success = dt->getParam( "status active", trackingStatus );
	if ( success )
	{
		// check the answer (possibilities: none, cal, mea, wait or err)
		if ( trackingStatus.compare( "none" ) != 0 && trackingStatus.compare( "err" ) != 0 )
		{
			if ( !dt->stopMeasurement() )
			{
				int errorOccured = dtrack2_error_to_console();
				if ( errorOccured )
					return errorOccured;
			}
		}
	}
	else
	{
		int errorOccured = dtrack2_error_to_console();
		if ( errorOccured )
			return errorOccured;
	}
	return 0;
}


/**
*	\brief Calls getParam function
*/
static int get_dtrack2_parameter( const std::string& someParameter )
{
	std::string receivedParameter;

	bool success = dt->getParam( someParameter, receivedParameter );
	if ( success )
		std::cout << receivedParameter << std::endl;
	else
	{
		int errorOccured = dtrack2_error_to_console();
		if ( errorOccured )
			return errorOccured;
	}
	return 0;
}


/**
*	\brief Calls setParam function
*/
static int set_dtrack2_parameter( const std::string& someParameter )
{
	if ( !dt->setParam( someParameter ) )
	{
		int errorOccured = dtrack2_error_to_console();
		if ( errorOccured )
			return errorOccured;
	}
	return 0;
}


/**
*	\brief Sends raw DTrack2 command
*/
static int send_dtrack2_command( const std::string& rawDtrack2Command )
{
	std::string dtrack2Response;

	int isSuccessful = dt->sendDTrack2Command( rawDtrack2Command, &dtrack2Response );
	if ( isSuccessful == 0 )
		std::cout << dtrack2Response << std::endl;
	else
	{
		int errorOccured = dtrack2_error_to_console();
		if ( errorOccured )
			return errorOccured;
	}
	return 0;
}


/**
*	\brief Processes commands from pipe/file
*/
static int process_command( std::string someCommand )
{
	// getParam (request parameter value); accepts commands without "dtrack2" in the beginning
	if ( someCommand.compare( 0, 4, "get " ) == 0 )
	{
		someCommand.erase( 0, 4 );
		int errorOccured = get_dtrack2_parameter( someCommand );
		if ( errorOccured )
			return errorOccured;
	}
	// getParam (request parameter value)
	else if ( someCommand.compare( 0, 12, "dtrack2 get " ) == 0 )
	{
		someCommand.erase( 0, 12 );
		int errorOccured = get_dtrack2_parameter( someCommand );
		if ( errorOccured )
			return errorOccured;
	}
	// setParam (change parameter value); accepts commands without "dtrack2" in the beginning
	else if ( someCommand.compare( 0, 4, "set " ) == 0 )
	{
		someCommand.erase( 0, 4 );
		int errorOccured = set_dtrack2_parameter( someCommand );
		if ( errorOccured )
			return errorOccured;
	}
	// setParam (change parameter value)
	else if ( someCommand.compare( 0, 12, "dtrack2 set " ) == 0 )
	{
		someCommand.erase( 0, 12 );
		int errorOccured = set_dtrack2_parameter( someCommand );
		if ( errorOccured )
			return errorOccured;
	}
	// raw DTrack2 command
	else
	{
		if ( someCommand.compare( 0, 8, "dtrack2 " ) != 0 )
		{
			// inserts "dtrack2" in the beginning of the string if it's missing
			someCommand = "dtrack2 " + someCommand;
		}
		int errorOccured = send_dtrack2_command( someCommand );
		if ( errorOccured )
			return errorOccured;
	}
	return 0;
}


/**
*	\brief Open file and check for commands
*/
static int open_file( const std::string& file_to_open )
{
	std::string readLine;
	std::ifstream readFile( file_to_open.c_str() );

	// returns the first occured error after DTrack2CLI is finished
	int firstErrorInFile = 0;

	// check if file opened correctly
	if ( readFile.is_open() )
	{
		// read commands in file (executed line by line)
		while ( std::getline( readFile, readLine ) )
		{
			if ( readLine.empty() )
				continue;

			// errors will be displayed, but DTrack2CLI won't stop
			int errorOccured = process_command( readLine );
			if ( ( firstErrorInFile == 0 ) && errorOccured )
				firstErrorInFile = errorOccured;
		}
		// close file
		readFile.close();

		int errorOccured = dtrack2_error_to_console();
		if ( errorOccured )
			return errorOccured;
	}
	// file couldn't be opened/found
	else
	{
		std::cerr << "Unable to open file" << std::endl;
		return ERR_OPEN_FILE;
	}

	if ( firstErrorInFile == 0 )
		return 0;
	else
		return firstErrorInFile;
}


/**
*	\brief Processes all commands in command line
*/
static int process_cmd_line_input( const int& argc, char** argv )
{
	// counter: checks the number of commands (acts as cursor)
	int nCommands = 2;

	// enables multiple input commands
	while ( argc >= nCommands )
	{
		// only occurs the first time this loop is executed
		if ( nCommands == 2 )
			++nCommands;

		// takes one argument from input
		std::string command( argv[ nCommands - 1 ] );

		// help
		if ( ( command.compare( "-h" ) == 0 ) ||
			( command.compare( "--help" ) == 0 ) ||
			( command.compare( "/?" ) == 0 ) )
		{
			++nCommands;
			show_help( argv[ 0 ] );
		}
		// start measurement
		else if ( command.compare( "-meastart" ) == 0 )
		{
			++nCommands;
			int errorOccured = start_measurement();
			if ( errorOccured )
				return errorOccured;
		}
		// stop measurement
		else if ( command.compare( "-meastop" ) == 0 )
		{
			++nCommands;
			int errorOccured = stop_measurement();
			if ( errorOccured )
				return errorOccured;
		}
		// shutdown controller
		else if ( command.compare( "-shutdown" ) == 0 )
		{
			// if shutdown doesn't work, look for an error
			if ( send_dtrack2_command( "dtrack2 system shutdown" ) )
			{
				int errorOccured = dtrack2_error_to_console();
				if ( errorOccured )
					return errorOccured;
			}
			return 0;
		}
		// getParam (request parameter value)
		else if ( command.compare( "-get" ) == 0 )
		{
			std::string dtrack2Get;

			dtrack2Get = argv[ nCommands ];
			nCommands = nCommands + 2;

			int errorOccured = get_dtrack2_parameter( dtrack2Get );
			if ( errorOccured )
				return errorOccured;
		}
		// setParam (change parameter value)
		else if ( command.compare( "-set" ) == 0 )
		{
			std::string dtrack2SetParam;
			std::string dtrack2SetValue;
			std::string dtrack2Set;

			dtrack2SetParam = argv[ nCommands ];
			dtrack2SetValue = argv[ nCommands + 1 ];
			nCommands = nCommands + 3;
			dtrack2Set = dtrack2SetParam + " " + dtrack2SetValue;

			int errorOccured = set_dtrack2_parameter( dtrack2Set );
			if ( errorOccured )
				return errorOccured;
		}
		// raw DTrack2 command
		else if ( command.compare( "-cmd" ) == 0 )
		{
			std::string dtrack2Cmd;

			dtrack2Cmd = argv[ nCommands ];
			nCommands = nCommands + 2;

			if ( dtrack2Cmd.compare( 0, 8, "dtrack2 " ) != 0 )
			{
				// inserts "dtrack2" in the beginning of the string if it's missing
				dtrack2Cmd = "dtrack2 " + dtrack2Cmd;
			}

			int errorOccured = send_dtrack2_command( dtrack2Cmd );
			if ( errorOccured )
				return errorOccured;
		}
		// read from file
		else if ( command.compare( "-f" ) == 0 )
		{
			std::string filename;

			filename = argv[ nCommands ];
			nCommands = nCommands + 2;

			int errorOccured = open_file( filename );
			if ( errorOccured )
				return errorOccured;
		}
		else
		{
			std::cout << "unknown error occured" << std::endl;
			return ERR_UNKNOWN;
		}
	}
	return 0;
}


/**
 * 	\brief Main.
 */
int main( int argc, char** argv )
{
	// check for correct usage
	if ( argc <= 1 )
	{
		show_help( argv[ 0 ] );
		return ERR_WRONG_USAGE;
	}

	// check if user wanted help
	std::string command( argv[ 1 ] );
	if ( ( command.compare( "-h" ) == 0 ) ||
	     ( command.compare( "--help" ) == 0 ) ||
	     ( command.compare( "/?" ) == 0 ) )
	{
		show_help( argv[ 0 ] );
		return 0;
	}
	
	// init library:
	dt = new DTrackSDK( ( const char * )argv [ 1 ], 0 );

	if ( !dt->isCommandInterfaceValid() ) 
	{
		std::cerr << "No connection to ART controller! Is \"" << argv[ 1 ] << 
		             "\" a valid controller hostname of ip address?" << std::endl;
		delete dt;
		return ERR_DTRACKSDK_INIT;
	}

	// runs DTrack2CLI without commands, enabling pipes or multiple commands (executed seperately)
	if ( argc == 2 ) 
	{
		// look for pipe
		std::string pipeCommand;

		// returns the first occured error after DTrack2CLI is finished
		int firstErrorInPipe = 0;

		// checks for different possible commands
		while ( std::getline( std::cin, pipeCommand ) ) 
		{
			// errors will be displayed, but DTrack2CLI won't stop
			int errorOccured = process_command( pipeCommand );
			if ( ( firstErrorInPipe == 0 ) && errorOccured )
				firstErrorInPipe = errorOccured;
		}

		if ( firstErrorInPipe != 0 )
		{
			delete dt;
			return firstErrorInPipe;
		}
	}
	// runs DTrack2CLI with entered commands
	else
	{
		// check input parameters
		int paramInputError = checkInput( argc, argv );
		if ( paramInputError )
		{
			delete dt;
			return paramInputError;
		}

		// processes all commands
		int someError = process_cmd_line_input( argc, argv );
		if ( someError )
		{
			delete dt;
			return someError;
		}
	}

	delete dt;
	return 0;
}

