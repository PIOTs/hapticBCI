
#include "Emotiv.h"
using namespace std;


static shared_data* p_sharedData;  // structure for sharing data between threads


// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToEmotiv(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// reset cognitive powers
void initEmotiv(void) {

	p_sharedData->cursorPos = 0;
	p_sharedData->cursorPosOneAgo = 0;
	p_sharedData->cursorVel = 0;
	p_sharedData->cursorVelOneAgo = 0;
	p_sharedData->cogRight = 0;
	p_sharedData->cogLeft = 0;
	p_sharedData->cogNeut = 0;

}

// plug in the socket to start listening to Emotiv
void updateEmotiv(void) {
    
    UdpListeningReceiveSocket socket(IpEndpointName(IpEndpointName::ANY_ADDRESS, PORT), &(p_sharedData->listener));
    socket.RunUntilSigInt();

}

