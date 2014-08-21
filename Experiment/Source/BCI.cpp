
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
    
    if (p_sharedData->bci == GTEC) {
        bool sending = false;
        bool receiving = false;
        
        // if not already done, open the sockets and streams
        if (!p_sharedData->sendSocket.is_open()) p_sharedData->sendSocket.open(SEND_SOCK);
        if (!p_sharedData->sendStream.is_open()) p_sharedData->sendStream.open(p_sharedData->sendSocket);
        if (!p_sharedData->recSocket.is_open()) p_sharedData->recSocket.open(REC_SOCK);
        if (!p_sharedData->recStream.is_open()) p_sharedData->recStream.open(p_sharedData->recSocket);
        
        // check that the connection with g.MOBIlab+ is established
        if (!p_sharedData->sendStream.is_open()) {
            sending = false;
            printf("\nUNABLE TO SEND DATA...");
        } else {
            sending = true;
            printf("\nSENDING DATA...");
        }
        if (!p_sharedData->recStream.is_open()) {
            receiving = false;
            printf("\nUNABLE TO RECEIVE DATA...");
        } else {
            receiving = true;
            printf("\nRECEIVING DATA...");
        }
        
        // keep trying to initialize until the connection is established
        while (!sending || !receiving) {
            cSleepMs(2500);
            initBCI();
        }
    }
    
}

// update state of BCI, either the Emotiv or g.MOBIlab+
void updateBCI(void) {
    
    // plug in the socket to start listening to the Emotiv
    if (p_sharedData->bci == EMOTIV) {
        UdpListeningReceiveSocket socket(IpEndpointName(IpEndpointName::ANY_ADDRESS, PORT), &(p_sharedData->listener));
        socket.RunUntilSigInt();
    }
    
    // (re)read from the g.MOBIlab+
    else if (p_sharedData->bci == GTEC) readFromGTec(p_sharedData->state, p_sharedData->recStream);

}

// update map holding state of g.MOBIlab+
bool readFromGTec(map<string, float> &state, sockstream &recStream) {
    
    int count = 0;
    
    // read state data while available
    while (recStream.rdbuf()->in_avail()) {
        string label;
        float value;
        recStream >> label >> value;
        recStream.ignore();
        if (!recStream) recStream.clear();
        state[label] = value;
        count++;
    }
    recStream.clear();
    
    // if we read and recorded data, return true
    if (count > 0) return true;
    else           return false;
    
}

// overwrite a desired state of g.MOBIlab+
void writeToGTec(string state, short value) {
    
    p_sharedData->sendStream << state << ' ' << value << endl;
    
}

// if necessary, close the connection with g.MOBIlab+ (NOTE: because the Emotiv socket is only locally declared in the update function above (does not work to declare it in "shared_Data.h"), there is no way to break it)
void closeBCI(void) {
    
    if (p_sharedData->bci == GTEC) {
        p_sharedData->recStream.close();
        p_sharedData->recStream.clear();
        p_sharedData->recSocket.close();
        p_sharedData->sendStream.close();
        p_sharedData->sendStream.clear();
        p_sharedData->sendStream.close();
    }
    
}
