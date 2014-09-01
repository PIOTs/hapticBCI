
#include "NeuroTouch.h"
using namespace std;


static const double mCursor = 5000.0;    // mass of cursor (scales cognitive "force" on cursor)
static const double gTecScalar = 0.0001; // to scale control signal output from BCI2000 module
static const double dt = 0.02;           // time between graphic updates [sec]
static const double phantomScalar = 2;   // to scale PHANTOM workspace to graphics workspace
static const double A = 0.15;            // amplitude of autonomous cursor movement
static const double pi = 3.14159;        // for conversion to radians/sec
static const double maxPos = 0.2;		 // to limit cursor movement
static const double Kpos = 2.5;          // position-based control gain
static const double Kvel = 1.5;          // velocity-based control gain

static shared_data* p_sharedData;  // structure for sharing data between threads


// initialize NeuroTouch
void initNeuroTouch(void) {
    
    p_sharedData->p_NeuroTouch->open();
	p_sharedData->p_NeuroTouch->initADC();
    p_sharedData->p_NeuroTouch->setForce(0,0);
    
    // prompt user to "zero" device
    printf("\nMove the skin-stretch tactor to home, then press any key to continue.\n");
    while (true) {
        if (_kbhit()) break;
    }
    p_sharedData->p_NeuroTouch->zeroEncoders();
    
    // initialize device loop timer
	p_sharedData->m_neurotouchLoopTimer.setTimeoutPeriodSeconds(LOOP_TIME);
	p_sharedData->m_neurotouchLoopTimer.start();
    
}

// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToNeuroTouch(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// haptic loop (NOTE: as the primary haptic device, NeuroTouch governs the start and end of simulation)
void updateNeuroTouch(void) {

    // initialize frequency counter
    p_sharedData->neurotouchFreqCounter.reset();
    
    // start simulation
    p_sharedData->simulationRunning = true;
    while(p_sharedData->simulationRunning) {
        
        // only update state of device if timer has expired
		if (p_sharedData->m_neurotouchLoopTimer.timeoutOccurred()) {
            
			p_sharedData->m_neurotouchLoopTimer.stop();

			// update cursor and device states
			updateCursor();
			p_sharedData->p_NeuroTouch->getPosition(p_sharedData->motorAPos, p_sharedData->motorBPos);
        
			// compute desired end-effector force and command this force to the device
			computeForce();
			p_sharedData->p_NeuroTouch->setForce(p_sharedData->eeForceDesX, p_sharedData->eeForceDesY);
        
			// update frequency counter
			p_sharedData->neurotouchFreqCounter.signal(1);

			p_sharedData->m_neurotouchLoopTimer.start(true);
		}

    }

    // signal end of simulation
    p_sharedData->simulationFinished = true;
    
}

// update position and velocity of cursor on screen based on designated input
void updateCursor(void) {
    
    // save old cursor state
    p_sharedData->cursorPosOneAgo = p_sharedData->cursorPos;
    p_sharedData->cursorVelOneAgo = p_sharedData->cursorVel;
    
    if (p_sharedData->input == BCI) {
        
        if (p_sharedData->bci == EMOTIV) {

            // query the Emotiv listener
            p_sharedData->listener.queryEmoState(p_sharedData->cogRight, p_sharedData->cogLeft, p_sharedData->cogNeut);
            
            // sum of cognitive powers (right, left, neutral) = "force" on cursor along X
            double cursorForce = p_sharedData->cogRight - p_sharedData->cogLeft;
            double cursorAcc = cursorForce / mCursor;
            
            // integrate (via Euler) acceleration to get new cursor velocity
            p_sharedData->cursorVel = p_sharedData->cursorVelOneAgo + cursorAcc * dt;
        }
        
        else if (p_sharedData->bci == GTEC) {

			// update g.MOBIlab+ state map (and simultaneously check for success)
			if (!readFromGTec(p_sharedData->state, p_sharedData->recStream)) printf("\nUNABLE TO UPDATE STATE FROM BCI2000");

			// query the map for the Y control signal (the one changing in BCI2000 CursorTask), which scales to X cursor velocity
            p_sharedData->controlSig = p_sharedData->state["Signal(1,0)"];
            p_sharedData->cursorVel = gTecScalar * p_sharedData->controlSig;
			printf("\nv = %f\n", p_sharedData->cursorVel);
        }
		
        // integrate (via Euler) velocity to get new cursor position
        p_sharedData->cursorPos = p_sharedData->cursorPosOneAgo + p_sharedData->cursorVel * dt;
        
    } else if (p_sharedData->input == PHANTOM) {
        
        // scale PHANTOM state for graphics workspace
        p_sharedData->cursorVel = phantomScalar * p_sharedData->phantomVel;
        p_sharedData->cursorPos = phantomScalar * p_sharedData->phantomPos;
        
    } else {
        
        // autonomous, sinusoidal input (i.e., cursor moves on its own)
        p_sharedData->cursorPos = A * sin(2.0 * pi * p_sharedData->autoFreq * p_sharedData->time->getCurrentTimeSeconds());
        p_sharedData->cursorVel = (A * 2.0 * pi * p_sharedData->autoFreq) * cos(2.0 * pi * p_sharedData->autoFreq * p_sharedData->time->getCurrentTimeSeconds());
    }

	// limit cursor movement if necessary
	if (p_sharedData->cursorPos > maxPos) {
		p_sharedData->cursorPos = maxPos;
		p_sharedData->cursorVel = 0;
	}
	if (p_sharedData->cursorPos < -maxPos) {
		p_sharedData->cursorPos = -maxPos;
		p_sharedData->cursorVel = 0;
	}
	 
}

// compute desired force at end effector (NOTE: add Y-force when extending task to 2 dimensions) and (if sensing) measure finger force
void computeForce(void) {
    
    switch (p_sharedData->controller) {
            
        // for safety
        case HAPTICS_OFF:
            p_sharedData->eeForceDesX = 0;
            p_sharedData->eeForceDesY = 0;
            break;
            
        // force scales with position, in same direction as cursor movement
        case POS_WITH_CURSOR:
            p_sharedData->eeForceDesX = Kpos * (p_sharedData->cursorPos);
            p_sharedData->eeForceDesY = 0;
            break;
            
        // force scales with position, in opposite direction as cursor movement
        case POS_ANTI_CURSOR:
            p_sharedData->eeForceDesX = -Kpos * (p_sharedData->cursorPos);
            p_sharedData->eeForceDesY = 0;
            break;
            
        // force scales with velocity, in same direction as cursor movement
        case VEL_WITH_CURSOR:
            p_sharedData->eeForceDesX = Kvel * (p_sharedData->cursorVel);
            p_sharedData->eeForceDesY = 0;
            break;
            
        // force scales with velocity, in opposite direction as cursor movement
        case VEL_ANTI_CURSOR:
            p_sharedData->eeForceDesX = -Kvel * (p_sharedData->cursorVel);
            p_sharedData->eeForceDesY = 0;
            break;
            
        // again, for safety
        default:
            p_sharedData->eeForceDesX = 0;
            p_sharedData->eeForceDesY = 0;
            break;
    }
    
    // if sensing, measure XYZ finger force
    if (p_sharedData->sensing) {
        int n = p_sharedData->g_ForceSensor.AcquireFTData();  // integer output indicates success/failure
        p_sharedData->g_ForceSensor.GetForceReading(p_sharedData->force);
    }
    
}

// safely close NeuroTouch
void closeNeuroTouch(void) {
    
    p_sharedData->p_NeuroTouch->close();

}
