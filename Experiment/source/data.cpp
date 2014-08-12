
#include "data.h"
using namespace std;


static char response;   // Y/N response to set-up questions
static save_data temp;  // for temporarily holding one time step of data (that is to be saved)

static shared_data* p_sharedData;  // structure for sharing data between threads


// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedData(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// set-up for simulation
void setup(void) {
    
    // initialize shared data
	p_sharedData->simulationRunning = false;
	p_sharedData->simulationFinished = false;
	p_sharedData->phantomPos = 0;
	p_sharedData->phantomVel = 0;
	p_sharedData->cogRight = 0;
	p_sharedData->cogLeft = 0;
	p_sharedData->cogNeut = 0;
    p_sharedData->controlSig = 0;
	p_sharedData->opMode = DEMO;
	p_sharedData->input = AUTO;
    p_sharedData->bci = EMOTIV;
	p_sharedData->controller = HAPTICS_OFF;
	p_sharedData->autoFreq = 0.02;
    p_sharedData->cursorPos = 0;
    p_sharedData->cursorPosOneAgo = 0;
    p_sharedData->cursorVel = 0;
    p_sharedData->cursorVelOneAgo = 0;
    p_sharedData->eeForceDesX = 0;
    p_sharedData->eeForceDesY = 0;
    p_sharedData->sensing = false;
    p_sharedData->force[3] = {0,0,0};
	p_sharedData->targetSide = RIGHT;
	p_sharedData->experimentState = START_UP;
    p_sharedData->blockNum = 0;
    p_sharedData->trialNum = 0;
	p_sharedData->trialSuccess = false;
	
	// create timers, PHANTOM device handler, and NeuroTouch device
    // NOTE: only use these constructors once (at beginning of main) to avoid pointer issues
	p_sharedData->time = new cPrecisionClock();
	p_sharedData->time->start(true);
    p_sharedData->timer = new cPrecisionClock();
    p_sharedData->p_phantomHandler = new cHapticDeviceHandler();
    p_sharedData->p_phantomHandler->getDevice(p_sharedData->p_Phantom, 0);  // 1st available haptic device
    p_sharedData->p_NeuroTouch = new cNeuroTouch();
    
    // ask for operating mode (defaults to demo)
    printf("\nIs this going to be an experiment?\n");
    cin >> response;
    if (response == 'y' || response == 'Y') {
        p_sharedData->opMode = EXPERIMENT;
    }
    // if not experiment, ask for input device (defaults to autonomous)
    else {
        printf("\nHow do you want to control?");
        printf("\n(0) auto, (1) BCI, (2) PHANTOM\n");
        cin >> response;
        // if BCI control, ask for BCI (defaults to Emotiv headset)
        if (response == '1') {
            p_sharedData->input = BCI;
            printf("\nWhich BCI are you using?");
            printf("\n(0) Emotiv headset, (1) g.MOBIlab+ EEG cap\n");
            cin >> response;
            if (response == '1') p_sharedData->bci = GTEC;
        }
        else if (response == '2') p_sharedData->input = PHANTOM;
    }
    
}

// save one time step of data to vector for current trial
void saveOneTimeStep(void) {
    
	save_data temp;
    
    // record individual parameters
    temp.d_blockNum = p_sharedData->blockNum;
    temp.d_trialNum = p_sharedData->trialNum;
    temp.d_trialSuccess = p_sharedData->trialSuccess;
    temp.d_targetSide = p_sharedData->targetSide;
    temp.d_cursorPos = p_sharedData->cursorPos;
    temp.d_cursorVel = p_sharedData->cursorVel;
    temp.d_timeElapsed = p_sharedData->timeElapsed;
    temp.d_cogRight = p_sharedData->cogRight;
    temp.d_cogLeft = p_sharedData->cogLeft;
    temp.d_cogNeut = p_sharedData->cogNeut;
    temp.d_controlSig = p_sharedData->controlSig;
    temp.d_eeForceDesX = p_sharedData->eeForceDesX;
    temp.d_eeForceDesY = p_sharedData->eeForceDesY;
    temp.d_motorAPos = p_sharedData->motorAPos;
    temp.d_motorBPos = p_sharedData->motorBPos;
    for (int i=0; i<3; i++) temp.d_force[i] = p_sharedData->force[i];
    
    // push into vector for current trial
	p_sharedData->trialData.push_back(temp);
    
}

// write data to file from current trial
void recordTrial(void) {
    
    // iterate over vector, writing one time step at a time
    for (vector<save_data>::iterator it = p_sharedData->trialData.begin() ; it != p_sharedData->trialData.end(); ++it) {
        fprintf(p_sharedData->outputFile,"%d %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f\n",
                it->d_blockNum,
                it->d_trialNum,
                it->d_trialSuccess,
                it->d_targetSide,
                it->d_cursorPos,
                it->d_cursorVel,
                it->d_timeElapsed,
                it->d_cogRight,
                it->d_cogLeft,
                it->d_cogNeut,
                it->d_controlSig,
                it->d_eeForceDesX,
                it->d_eeForceDesY,
                it->d_motorAPos,
                it->d_motorBPos,
                it->d_force[0],
                it->d_force[1],
                it->d_force[2]);
    }
    
    // clear vector for next segment and signal that recording is done
    p_sharedData->trialData.clear();
    
}
