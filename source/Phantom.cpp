
#include "Phantom.h"
using namespace chai3d;
using namespace std;


cVector3d pos;  // temporary variable for PHANTOM position (only need X)
cVector3d vel;  // temporary variable for PHANTOM velocity (only need X)

static shared_data* p_sharedData;  // structure for sharing data between threads


// initialize the PHANTOM device
void initPhantom(void) {
    
    // open and calibrate the PHANTOM
    p_sharedData->p_Phantom->open();
    p_sharedData->p_Phantom->calibrate();
    
    // initialize device loop timer
	p_sharedData->m_phantomLoopTimer.setTimeoutPeriodSeconds(LOOP_TIME);
	p_sharedData->m_phantomLoopTimer.start();
    
}

// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToPhantom(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// get PHANTOM state and update shared data
void updatePhantom(void) {

    // initialize frequency counter
    p_sharedData->phantomFreqCounter.reset();

    while(p_sharedData->simulationRunning) {

        if (p_sharedData->m_phantomLoopTimer.timeoutOccurred()) {
            
            p_sharedData->m_phantomLoopTimer.stop();

            if (p_sharedData->input == PHANTOM) {
    
                // get PHANTOM position and velocity vectors
                p_sharedData->p_Phantom->getPosition(pos);
                p_sharedData->p_Phantom->getLinearVelocity(vel);
    
                // extract X elements from vectors
                p_sharedData->phantomPos = pos.y();
                p_sharedData->phantomVel = vel.y();
                
                // update frequency counter
                p_sharedData->phantomFreqCounter.signal(1);
            }
            
            p_sharedData->m_phantomLoopTimer.start(true);
            Sleep(1);
        }
    }

}

// safely close the PHANTOM device
void closePhantom(void) {
    
    p_sharedData->p_Phantom->close();
    
}
