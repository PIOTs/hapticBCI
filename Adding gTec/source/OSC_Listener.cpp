#include "OSC_Listener.h"
#include <stdlib.h>
#include <stdio.h>

using namespace std;
// Required Library inclusion to work with oscpack library
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <cstring>

#include <cstdio>
#include <cassert>
#include <windows.h>
#include <stdio.h>
#include <queue>
#include "OscReceivedElements.h"
#include "OscPacketListener.h"
#include "UdpSocket.h"
#include "cNeuroTouch.h"
#include "shared_Data.h"
#include <conio.h>


#define MAIN
//#define TEST_SET_FORCE

//Port definition: should be the same port number that Mind Your OSCs is broadcasting on
#define PORT 7400

// DEBUG FLAG
//#define DEBUG_PRINTF
//#define TEST_EPOC_FORCE

// Module Level Defines
static float CogLeft = 0; // initialize cognitive power left to zero
static float CogRight = 0; // initialize cognitive power right to zero
static float CogNeutral = 0; // initialize cognitive power neutral to zero
static int Rx_FLAG = 0;

cNeuroTouch device;

OSC_Listener::OSC_Listener(void)
{

}


void OSC_Listener::queryEmoState(float& Cog_Right, float& Cog_Left, float& Cog_Neutral){
	Cog_Right = CogRight;
	Cog_Left  = CogLeft;
	Cog_Neutral = CogNeutral;


}




OSC_Listener::~OSC_Listener(void)
{
}


// Function to process the messages recieved on the desired port
    void OSC_Listener::ProcessMessage( const osc::ReceivedMessage& m, 
				const IpEndpointName& remoteEndpoint )
    {
		//printf("PROCESS\n\n");
        (void) remoteEndpoint; // suppress unused parameter warning

        try{
            
            // If a message is recieved, decode it
            if( std::strcmp( m.AddressPattern(), "/COG/LEFT" ) == 0 ){ // if the adress matches COG LEFT
                
				// Recieve the argument of the stream and store it into args
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream(); 
            
				// store the recieved message (cognitive magnitude) into a variable
                args >> CogLeft >>  osc::EndMessage;
				CogRight = 0;
				CogNeutral = 0;
                
				#ifdef DEBUG_PRINTF
                std::cout << "Recieved /COG/LEFT pattern with contents:     "
                    << CogLeft <<"\n\n";
				#endif
            
			}else if( std::strcmp( m.AddressPattern(), "/COG/RIGHT" ) == 0 ){ // else if the address matches COG RIGHT
				
				// Recieve the argument of the stream and store it into args
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream(); 
            
				// store the recieved message (cognitive magnitude) into a variable
                args >> CogRight >>  osc::EndMessage;
				CogLeft = 0;
				CogNeutral = 0;
                
				#ifdef DEBUG_PRINTF
                std::cout << "Recieved /COG/RIGHT pattern with contents:     "
                    << CogRight <<"\n\n";
				#endif

			}else if( std::strcmp( m.AddressPattern(), "/COG/NEUTRAL" ) == 0 ){ // else if the address matches COG Neutral
				
				// Recieve the argument of the stream and store it into args
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream(); 
            
				// store the recieved message (cognitive magnitude) into a variable
                args >> CogNeutral >>  osc::EndMessage;
				CogRight = 0;
				CogLeft = 0;
                
				#ifdef DEBUG_PRINTF
                std::cout << "Recieved /COG/NEUTRAL pattern with contents:     "
                    << CogNeutral <<"\n\n";
				#endif
            
            }



        }catch( osc::Exception& e ){ // catch any errors
            // any parsing errors such as unexpected argument types, or 
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: "
                << m.AddressPattern() << ": " << e.what() << "\n";
        }


		
		// Test The Set Force
#ifdef TEST_EPOC_FORCE
		// Set forces to the NeutoTouch, make instance of neurotouch
		
		static float Force_desired = 0;
		if( abs(CogRight) > abs(CogLeft)) Force_desired = CogRight;
		else Force_desired = -1*CogLeft;
		device.setForce(-1*Force_desired,0);
		printf("Force Desired: %f\n\n", Force_desired);

#endif


    }