
#include "BCI.h"
using namespace std;


static shared_data* p_sharedData;  // structure for sharing data between threads


// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToBCI(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// initialize BCI state (plus any associated) data and (if necessary) open a connection with the g.MOBIlab+
void initBCI(void) {

	// reset BCI state
    p_sharedData->state.clear();
    p_sharedData->cogRight = 0;
	p_sharedData->cogLeft = 0;
	p_sharedData->cogNeut = 0;
    
    // reset the kinematic variables controlled by the BCI state
    p_sharedData->cursorPos = 0;
	p_sharedData->cursorPosOneAgo = 0;
	p_sharedData->cursorVel = 0;
	p_sharedData->cursorVelOneAgo = 0;
    
    // if using g.MOBIlab+, open the sockets and streams
    if (p_sharedData->bci == GTEC) {
        p_sharedData->sendSocket.open(SEND_SOCK);
        p_sharedData->sendStream.open(p_sharedData->sendSocket);
        p_sharedData->recSocket.open(REC_SOCK);
        p_sharedData->recStream.open(p_sharedData->recSocket);
    }

}

// update state of BCI, either the Emotiv or g.MOBIlab+
void updateBCI(void) {
    
    // plug in the socket to start listening to the Emotiv
    if (p_sharedData->bci == EMOTIV) {
        UdpListeningReceiveSocket socket(IpEndpointName(IpEndpointName::ANY_ADDRESS, PORT), &(p_sharedData->listener));
        socket.RunUntilSigInt();
    }
    
    // reread from the g.MOBIlab+
    else if (p_sharedData->bci == GTEC) readFromGTec(p_sharedData->state, p_sharedData->recStream);

}

// update map holding state of g.MOBIlab+
bool readFromGTec(map<string, float> &state, sockstream &recStream) {
    
    int count = 0;
    
    // read state data while available
    while (recConnection.rdbuf()->in_avail()) {
        string label;
        float value;
        recConnection >> label >> value;
        recConnection.ignore();
        if (!recConnection) recConnection.ignore();
        state[label] = value;
        count++;
    }
    
    // if we read and recorded data, return true
    if (count > 0) return true;
    else           return false;
    
}
