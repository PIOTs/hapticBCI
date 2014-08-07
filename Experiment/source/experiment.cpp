
#include "experiment.h"
using namespace chai3d;
using namespace std;


static const int trialsPerBlock = 20;             // trials per control paradigm
static const int trialTime = 20;                  // max time per trial or washout [sec]
static const int breakTime = 180;                 // break time [sec]
static const int preblockTime = 10;               // time to display message [sec]
static const int recordTime = 5;                  // time to record data [sec]
static const int targetSides[2] = {RIGHT, LEFT};  // only experiment parameter

static int subjectNum;           // subject number
static int control;              // subject's control paradigm of choice
static char filename[100];       // output filename
static int nextExperimentState;  // so state machine knows where to go next

static shared_data* p_sharedData;  // structure for sharing data between threads


static float loopTime = 0.001;  // in seconds

// point p_sharedData to sharedData, which is the data shared between all threads
void linkSharedDataToExperiment(shared_data& sharedData) {
    
    p_sharedData = &sharedData;
    
}

// set-up for experiment
void initExperiment(void) {
    
    // get subject number and control paradigm
    printf("\nEnter subject number: ");
    cin >> subjectNum;
    printf("\nEnter control paradigm number: ");
    cin >> control;
    printf("\n");
    
    // generate filename and open file for writing
    sprintf(filename, "Subj_%dCtrl_%d.dat", subjectNum, control);
    p_sharedData->outputFile = fopen(filename,"w");
    fprintf(p_sharedData->outputFile, "Block, Trial, Success, TargetSide, CursorPos, CursorVel, TimeElapsed, CogRight, CogLeft, CogNeut, eeForceDesX, eeForceDesY, MotorAPos, MotorBPos\n");
    
    // enter start-up mode, with force feedback off for safety
    p_sharedData->controller = HAPTICS_OFF;
    p_sharedData->experimentState = START_UP;
    p_sharedData->message = "Welcome to NeuroTouch.";

	//Darrel Addition, need to change input type to EMOTIV otherwise it will run the DEMO graphics 
	p_sharedData->input = EMOTIV;

	// initialize loop timer
	p_sharedData->m_expLoopTimer.setTimeoutPeriodSeconds(loopTime);
	p_sharedData->m_expLoopTimer.start(true);
    
}

// experiment state machine (only entered if in experiment mode)
void updateExperiment(void) {
   
    while (p_sharedData->simulationRunning) {

		        
	    if (p_sharedData->m_expLoopTimer.timeoutOccurred())
		{
			// Stop the loop timer
			p_sharedData->m_expLoopTimer.stop();

			if (p_sharedData->opMode == EXPERIMENT) {
            
				switch (p_sharedData->experimentState) {
                    
					case START_UP:
                    
						// wait for a keypress
						while(true){
							if (_kbhit()) {
                        
								// ready subject for 1st block of trials
								nextExperimentState = NO_HAPTICS_1;
								p_sharedData->blockNum = 1;
								p_sharedData->message = "Beginning Block 1 in " + to_string(static_cast<long long>(preblockTime)) + " seconds.";
                        
								// set/start timer (from zero)
								p_sharedData->timer->setTimeoutPeriodSeconds(preblockTime);
								p_sharedData->timer->start(true);
								p_sharedData->experimentState = PREBLOCK;
								break;
							}
						}

						break;
                    
					case NO_HAPTICS_1:
                    
						// save data from time step
						p_sharedData->timeElapsed = p_sharedData->timer->getCurrentTimeSeconds();
						saveOneTimeStep();
                    
						// finished with 1st block
						if (p_sharedData->trialNum > trialsPerBlock) {
                        
							// give subject a break before 2nd block
							nextExperimentState = HAPTICS_1;
							p_sharedData->message = "Relax your mind.";
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(breakTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = BREAK;
						}
                    
						// successful trial (i.e., cursor reached target in time)
						if ( ((p_sharedData->cursorPos) >= (TARGET_DIST) ) && (p_sharedData->targetSide == RIGHT)  ||
							 ((p_sharedData->cursorPos) <= (-1*TARGET_DIST) ) && (p_sharedData->targetSide == LEFT)) {
                        
							p_sharedData->trialSuccess = true;
							p_sharedData->message = "Success.";
                        
							// start recording trial data
							saveOneTimeStep();
							recordTrial();
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = RECORD;
						}
                    
						// unsuccessful trial (i.e., time expired)
						if (p_sharedData->timer->timeoutOccurred()) {
                        
							p_sharedData->trialSuccess = false;
							p_sharedData->message = "Time expired.";
                        
							// start recording trial data
							saveOneTimeStep();
							recordTrial();
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = RECORD;
						}
						break;
                    
					case HAPTICS_1:
                    
						// save data from time step
						p_sharedData->timeElapsed = p_sharedData->timer->getCurrentTimeSeconds();
						saveOneTimeStep();
                    
						// finished with 2nd block
						if (p_sharedData->trialNum > trialsPerBlock) {
                        
							// turn off force feedback
							p_sharedData->controller = HAPTICS_OFF;
                        
							// give subject a break before 3rd block
							nextExperimentState = NO_HAPTICS_2;
							p_sharedData->message = "Relax your mind.";
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(breakTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = BREAK;
						}
                    
						// successful trial (i.e., cursor reached target in time)
						if ( ((p_sharedData->cursorPos) >= (TARGET_DIST) ) && (p_sharedData->targetSide == RIGHT)  ||
							 ((p_sharedData->cursorPos) <= (-1*TARGET_DIST) ) && (p_sharedData->targetSide == LEFT)) {
                        
							p_sharedData->trialSuccess = true;
							p_sharedData->message = "Success.";
                        
							// turn off force feedback
							p_sharedData->controller = HAPTICS_OFF;
                        
							// start recording trial data
							saveOneTimeStep();
							recordTrial();
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = RECORD;
						}
                    
						// unsuccessful trial (i.e., time expired)
						if (p_sharedData->timer->timeoutOccurred()) {
                        
							p_sharedData->trialSuccess = false;
							p_sharedData->message = "Time expired.";
                        
							// turn off force feedback
							p_sharedData->controller = HAPTICS_OFF;
                        
							// start recording trial data
							saveOneTimeStep();
							recordTrial();
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = RECORD;
						}
						break;
                    
					case NO_HAPTICS_2:
                    
						// save data from time step
						p_sharedData->timeElapsed = p_sharedData->timer->getCurrentTimeSeconds();
						saveOneTimeStep();
                    
						// finished with 3rd block
						if (p_sharedData->trialNum > trialsPerBlock) {
                        
							// give subject a break before 4th block
							nextExperimentState = HAPTICS_2;
							p_sharedData->message = "Relax your mind.";
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(breakTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = BREAK;
						}
                    
						// successful trial (i.e., cursor reached target in time)
					   if ( ((p_sharedData->cursorPos) >= (TARGET_DIST) ) && (p_sharedData->targetSide == RIGHT)  ||
							 ((p_sharedData->cursorPos) <= (-1*TARGET_DIST) ) && (p_sharedData->targetSide == LEFT)) {
                        
							p_sharedData->trialSuccess = true;
							p_sharedData->message = "Success.";
                        
							// start recording trial data
							saveOneTimeStep();
							recordTrial();
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = RECORD;
						}
                    
						// unsuccessful trial (i.e., time expired)
						if (p_sharedData->timer->timeoutOccurred()) {
                        
							p_sharedData->trialSuccess = false;
							p_sharedData->message = "Time expired.";                        
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = RECORD;

							// start recording trial data
							saveOneTimeStep();
							recordTrial();
						}
						break;
                    
					case HAPTICS_2:
                    
						// save data from time step
						p_sharedData->timeElapsed = p_sharedData->timer->getCurrentTimeSeconds();
						saveOneTimeStep();
                    
						// finished with last block
						if (p_sharedData->trialNum > trialsPerBlock) {
                        
							// turn off force feedback
							p_sharedData->controller = HAPTICS_OFF;
                        
							// thank subject and terminate experiment
							p_sharedData->message = "Thank you.";
							p_sharedData->experimentState = THANKS;
							closeExperiment();
						}
                    
						// successful trial (i.e., cursor reached target in time)
						if ( ((p_sharedData->cursorPos) >= (TARGET_DIST) ) && (p_sharedData->targetSide == RIGHT)  ||
							 ((p_sharedData->cursorPos) <= (-1*TARGET_DIST) ) && (p_sharedData->targetSide == LEFT)) {
                        
							p_sharedData->trialSuccess = true;
							p_sharedData->message = "Success.";
                        
							// turn off force feedback
							p_sharedData->controller = HAPTICS_OFF;
                        
							// start recording trial data
							saveOneTimeStep();
							recordTrial();
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = RECORD;
						}
                    
						// unsuccessful trial (i.e., time expired)
						if (p_sharedData->timer->timeoutOccurred()) {
                        
							p_sharedData->trialSuccess = false;
							p_sharedData->message = "Time expired.";
                        
							// turn off force feedback
							p_sharedData->controller = HAPTICS_OFF;
                        
							// start recording trial data
							saveOneTimeStep();
							recordTrial();
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(recordTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = RECORD;
						}
						break;
                    
					case PREBLOCK:
                    
						// wait for time to expire
						if (p_sharedData->timer->timeoutOccurred()) {
                        
							// prep for 1st trial
							p_sharedData->trialNum = 1;
							p_sharedData->targetSide = targetSides[rand() % 2];
							p_sharedData->cursorPos = 0;
							p_sharedData->cursorPosOneAgo = 0;
							p_sharedData->cursorVel = 0;
							p_sharedData->cursorVelOneAgo = 0;
                        
							// set control paradigm
							if (nextExperimentState == HAPTICS_1 || nextExperimentState == HAPTICS_2) p_sharedData->controller = control;
							else                                                                      p_sharedData->controller = HAPTICS_OFF;
                        
							// set/start timer (from zero) and begin block of trials
							p_sharedData->timer->setTimeoutPeriodSeconds(trialTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = nextExperimentState;
						}
						break;
                    
					case BREAK:
                    
						// break over
						if (p_sharedData->timer->timeoutOccurred()) {
                        
							// ready subject for next block of trials
							(p_sharedData->blockNum)++;
							p_sharedData->message = "Beginning Block " + to_string(static_cast<long long>(p_sharedData->blockNum)) + " in " + to_string(static_cast<long long>(preblockTime)) + " seconds.";
                        
							// set/start timer (from zero)
							p_sharedData->timer->setTimeoutPeriodSeconds(preblockTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = PREBLOCK;
						}
						break;
                    
					case RECORD:
                    
						// wait for time to expire (more than enough to record data)
						if (p_sharedData->timer->timeoutOccurred()) {
                        
							// prep for next trial
							(p_sharedData->trialNum)++;
							p_sharedData->trialSuccess = false;
							p_sharedData->targetSide = targetSides[rand() % 2];
							p_sharedData->cursorPos = 0;
							p_sharedData->cursorPosOneAgo = 0;
							p_sharedData->cursorVel = 0;
							p_sharedData->cursorVelOneAgo = 0;
                        
							// set control paradigm
							if (nextExperimentState == HAPTICS_1 || nextExperimentState == HAPTICS_2) p_sharedData->controller = control;
							else                                                                      p_sharedData->controller = HAPTICS_OFF;
                        
							// set/start timer (from zero) and return to block
							p_sharedData->timer->setTimeoutPeriodSeconds(trialTime);
							p_sharedData->timer->start(true);
							p_sharedData->experimentState = nextExperimentState;
						}
						break;
                    
					case THANKS:
						break;
                    
					default:
						break;
				}
			}
			p_sharedData->m_expLoopTimer.start(true); //restart the timer
		}
    }
     
}

// terminate the experiment
void closeExperiment(void) {
    
    p_sharedData->experimentState = THANKS;
    if (p_sharedData->outputFile != NULL) fclose(p_sharedData->outputFile);
    
}
