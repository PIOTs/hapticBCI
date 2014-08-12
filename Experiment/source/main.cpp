
/***********************************************************
 Darrel Deo & Sean Sketch
 ME 327 - Design and Control of Haptic Systems
 Spring 2014
 NeuroTouch - skin-stretch haptic feedback for
 improved control of brain-computer
 interfaces
 
 This code runs a user study to test the effectiveness
 of 4 control paradigms for the NeuroTouch haptic inter-
 face. It reads signals from the Emotiv EPOC neuroheadset
 via Mind Your OSCs* or the g.tec g.MOBIlab+ via BCI2000,
 uses these signals to control the cursor in a Fitt's
 Law-type task, and ultimately translates the cursor's
 position/velocity (depending on the current control
 paradigm) into an appropriate skin-stretch signal. All
 signals are one-dimensional (i.e., left/right).
 
 * Alternatively, input can come from a PHANTOM haptic
 device, such as the OMNI or Premium. This is useful
 for demos.
 
 Acknowledgements: Z.F. Quek, A. Stanley, S. Schorr, T. Gibo
 ************************************************************/

//----------
// Includes
//----------

#include <Windows.h>
#include <assert.h>
#include <cstdio>
#include <stdio.h>
#include <string.h>
#include "BCI.h"
#include "OSC_Listener.h"
#include "Phantom.h"
#include "NeuroTouch.h"
#include "cForceSensor.h"
#include "cATIForceSensor.h"
#include "cDaqHardwareInterface.h"
#include "chai3d.h"
#include "graphics.h"
#include "data.h"
#include "shared_Data.h"
using namespace chai3d;
using namespace std;


//------------------------
// Variables & Structures
//------------------------

// threads and the data shared between them (NOTE: graphics is threaded separate from the CThread architecture)
cThread* bciThread;
cThread* phantomThread;
cThread* neurotouchThread;
cThread* experimentThread;
shared_data sharedData;


//---------------
// Main Function
//---------------
int main(int argc, char* argv[]) {
	
    // set up simulation with user input
    linkSharedData(sharedData);
    setup();

    // create threads
    cThread* bciThread = new cThread();
    cThread* phantomThread = new cThread();
    cThread* neurotouchThread = new cThread();
    cThread* experimentThread = new cThread();
    
    // give each thread access to shared data
    linkSharedDataToBCI(sharedData);
    linkSharedDataToPhantom(sharedData);
    linkSharedDataToNeuroTouch(sharedData);
    linkSharedDataToExperiment(sharedData);
    linkSharedDataToGraphics(sharedData);
    
    // initialize devices
	if (sharedData.input == BCI)     initBCI();
    if (sharedData.input == PHANTOM) initPhantom();	
    initNeuroTouch();
    
	// initialize force sensor
	sharedData.g_ForceSensor.Set_Calibration_File_Loc(FS_CALIB);
	sharedData.g_ForceSensor.Initialize_Force_Sensor(FS_INIT);
	cSleepMs(1000);
	sharedData.g_ForceSensor.Zero_Force_Sensor();

    // initialize experiment or demo (default)
    if(sharedData.opMode == EXPERIMENT) initExperiment();

    // initialize graphics
    initGraphics(argc, argv);
    
    // display keyboard control options
    printf("\n\n*********************\n");
	printf("M = operating mode toggle (experiment vs. demo)\n");
	printf("I = input device toggle for demo mode (Emotiv vs. PHANTOM vs. auto)\n");
    printf("S = force sensing toggle (ON/OFF)");
    printf("C = controller toggle\n");
    printf("O = increase speed of autonomous cursor\n");
    printf("L = decrease speed of autonomous cursor\n");
	printf("F = fullscreen toggle\n");
	printf("Q/ESC = quit\n");
	printf("*********************\n\n");   	
	
    // start threads
	neurotouchThread->start(updateNeuroTouch, CTHREAD_PRIORITY_HAPTICS);  // highest priority
    bciThread->start(updateEmotiv, CTHREAD_PRIORITY_HAPTICS);
    phantomThread->start(updatePhantom, CTHREAD_PRIORITY_GRAPHICS);
    experimentThread->start(updateExperiment, CTHREAD_PRIORITY_GRAPHICS);
    glutTimerFunc(50, graphicsTimer, 0);
    glutMainLoop();
    
    // close everything
    close();

    // exit
    return 0;
}
