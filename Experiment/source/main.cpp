
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
 via Mind Your OSCs*, uses these signals to control the
 cursor in a Fitt's Law-type task, and ultimately trans-
 lates the cursor's position/velocity (depending on the
 current control paradigm) into an appropriate skin-
 stretch signal. All signals are one-dimensional (i.e.,
 left/right).
 
 * Alternatively, input can come from a PHANTOM haptic
 device, such as the OMNI or Premium. This is useful
 for demos.
 
 Acknowledgements: Z.F. Quek, A. Stanley, S. Schorr, T. Gibo
 ************************************************************/

//----------
// Includes
//----------


#include "chai3d.h"
#include "Phantom.h"
#include "OSC_Listener.h"
#include "shared_Data.h"
#include "data.h"
#include "Emotiv.h"
#include "NeuroTouch.h"
#include "graphics.h"
#include <string.h>
#include "cATIForceSensor.h"
#include "cForceSensor.h"
#include "cDaqHardwareInterface.h"
#include <Windows.h>
#include <assert.h>
#include <cstdio>
#include <stdio.h>

using namespace chai3d;
using namespace std;


//------------------------
// Variables & Structures
//------------------------

// threads and the data shared between them (NOTE: graphics is threaded separate from the CThread architecture)
cThread* emotivThread;
cThread* phantomThread;
cThread* neurotouchThread;
cThread* experimentThread;
shared_data sharedData;

// create instance of force sensor
//cForceSensor g_ForceSensor;

// Set location for the calibration file
std::string calibrationFileLocation = "C:\CalibrationFiles\FT13574.cal";


//---------------
// Main Function
//---------------
#define MAIN
#ifdef MAIN
int main(int argc, char* argv[])
{
	
    // set up simulation with user input
    linkSharedData(sharedData);
    setup();

    // create threads
    cThread* emotivThread = new cThread();
    cThread* phantomThread = new cThread();
    cThread* neurotouchThread = new cThread();
    cThread* experimentThread = new cThread();
    
    // give each thread access to shared data
    linkSharedDataToEmotiv(sharedData);
    linkSharedDataToPhantom(sharedData);
    linkSharedDataToNeuroTouch(sharedData);
    linkSharedDataToExperiment(sharedData);
    linkSharedDataToGraphics(sharedData);
    
    // initialize devices
	if (sharedData.input == EMOTIV)  initEmotiv();	
    if (sharedData.input == PHANTOM) initPhantom();	
    initNeuroTouch();
    
		// Initialize force sensor 
	// 1) set calibration file for specific force sensor 
	sharedData.g_ForceSensor.Set_Calibration_File_Loc("C:/CalibrationFiles/FT13574.cal");
	
	// 2) initialize from file 
	sharedData.g_ForceSensor.Initialize_Force_Sensor("Dev1/ai0:5");
	
	// 3) sleep for 1 second
	Sleep(1000); 
	// 4) set all force to zero (tare force sensor)
	sharedData.g_ForceSensor.Zero_Force_Sensor();
	

    // initialize experiment or demo (default)
    if(sharedData.opMode == EXPERIMENT) initExperiment();

    // initialize graphics
    initGraphics(argc, argv);
    
    // display keyboard control options
    printf("\n\n*********************\n");
	printf("M = operating mode toggle (experiment vs. demo)\n");
	printf("I = input device toggle for demo mode (Emotiv vs. PHANTOM vs. auto)\n");
    printf("C = controller toggle\n");
    printf("O = increase speed of autonomous cursor\n");
    printf("L = decrease speed of autonomous cursor\n");
	printf("F = fullscreen toggle\n");
	printf("Q/ESC = quit\n");
	printf("*********************\n\n");   	
	
    // start threads
	neurotouchThread->start(updateNeuroTouch, CTHREAD_PRIORITY_HAPTICS);  // highest priority
    emotivThread->start(updateEmotiv, CTHREAD_PRIORITY_HAPTICS);
   //phantomThread->start(updatePhantom, CTHREAD_PRIORITY_GRAPHICS);
    experimentThread->start(updateExperiment, CTHREAD_PRIORITY_GRAPHICS);
    glutTimerFunc(50, graphicsTimer, 0);
    glutMainLoop();


    // close everything
    close();

    // exit
    return 0;
}
#endif // 



//#define TEST_QUAD04
#ifdef TEST_QUAD04

int main(int argc, char* argv[]){
	// set up simulation with user input
    linkSharedData(sharedData);
    setup();

    // create threads
    cThread* emotivThread = new cThread();
    cThread* phantomThread = new cThread();
    cThread* neurotouchThread = new cThread();
    cThread* experimentThread = new cThread();
    
    // give each thread access to shared data
    linkSharedDataToEmotiv(sharedData);
    linkSharedDataToPhantom(sharedData);
    linkSharedDataToNeuroTouch(sharedData);
    linkSharedDataToExperiment(sharedData);
    linkSharedDataToGraphics(sharedData);
    

    // initialize devices
	if (sharedData.input == EMOTIV)  initEmotiv();	
    if (sharedData.input == PHANTOM) initPhantom();	
    initNeuroTouch();
  

    // initialize experiment or demo (default)
    if(sharedData.opMode == EXPERIMENT) initExperiment();

    // initialize graphics
    initGraphics(argc, argv);
    
    // display keyboard control options
    printf("\n\n*********************\n");
	printf("M = operating mode toggle (experiment vs. demo)\n");
	printf("I = input device toggle for demo mode (Emotiv vs. PHANTOM vs. auto)\n");
    printf("C = controller toggle\n");
    printf("O = increase speed of autonomous cursor\n");
    printf("L = decrease speed of autonomous cursor\n");
	printf("F = fullscreen toggle\n");
	printf("Q/ESC = quit\n");
	printf("*********************\n\n");   	
	



    // start threads
	neurotouchThread->start(updateNeuroTouch, CTHREAD_PRIORITY_HAPTICS);  // highest priority
    emotivThread->start(updateEmotiv, CTHREAD_PRIORITY_HAPTICS);
   //phantomThread->start(updatePhantom, CTHREAD_PRIORITY_GRAPHICS);
    experimentThread->start(updateExperiment, CTHREAD_PRIORITY_GRAPHICS);
    glutTimerFunc(50, graphicsTimer, 0);
    glutMainLoop();



    // close everything
    close();

    // exit
    return 0;




}



#endif // TEST_QUAD04





//#define TEST_NIDAQ_FT
#ifdef TEST_NIDAQ_FT

int main(int argc, char* argv[]){
	// set up simulation with user input
    linkSharedData(sharedData);
    setup();

    // create threads
    cThread* emotivThread = new cThread();
    cThread* phantomThread = new cThread();
    cThread* neurotouchThread = new cThread();
    cThread* experimentThread = new cThread();
    
    // give each thread access to shared data
    linkSharedDataToEmotiv(sharedData);
    linkSharedDataToPhantom(sharedData);
    linkSharedDataToNeuroTouch(sharedData);
    linkSharedDataToExperiment(sharedData);
    linkSharedDataToGraphics(sharedData);
    

    // initialize devices
	if (sharedData.input == EMOTIV)  initEmotiv();	
    if (sharedData.input == PHANTOM) initPhantom();	
    initNeuroTouch();
  
	// Initialize force sensor 
	// 1) set calibration file for specific force sensor 
	sharedData.g_ForceSensor.Set_Calibration_File_Loc("C:/CalibrationFiles/FT13574.cal");
	printf("Set Location\n\n");
	// 2) initialize from file 
	sharedData.g_ForceSensor.Initialize_Force_Sensor("Dev1/ai0:5");
	printf("Initialized\n");
	// 3) sleep for 1 second
	Sleep(1000); 
	// 4) set all force to zero (tare force sensor)
	sharedData.g_ForceSensor.Zero_Force_Sensor();


    // initialize experiment or demo (default)
    if(sharedData.opMode == EXPERIMENT) initExperiment();

    // initialize graphics
    initGraphics(argc, argv);
    
    // display keyboard control options
    printf("\n\n*********************\n");
	printf("M = operating mode toggle (experiment vs. demo)\n");
	printf("I = input device toggle for demo mode (Emotiv vs. PHANTOM vs. auto)\n");
    printf("C = controller toggle\n");
    printf("O = increase speed of autonomous cursor\n");
    printf("L = decrease speed of autonomous cursor\n");
	printf("F = fullscreen toggle\n");
	printf("Q/ESC = quit\n");
	printf("*********************\n\n");   	
	

	double measuredForce[3];
	double forceData[3] = {0,0,0};
	for(;;){
		// display the forces
		// get force sensor data 
	int forceSensorData = sharedData.g_ForceSensor.AcquireFTData();
	sharedData.g_ForceSensor.GetForceReading(forceData);
	
	measuredForce[0] = forceData[0]; 
	measuredForce[1] = forceData[1]; 
	measuredForce[2] = forceData[2]; 

	printf("\nX:    %f\nY:    %f\nZ:    %f\n", measuredForce[0], measuredForce[1], measuredForce[2]);
	Sleep(5000);


	}
	
    // start threads
	neurotouchThread->start(updateNeuroTouch, CTHREAD_PRIORITY_HAPTICS);  // highest priority
    emotivThread->start(updateEmotiv, CTHREAD_PRIORITY_HAPTICS);
   //phantomThread->start(updatePhantom, CTHREAD_PRIORITY_GRAPHICS);
    experimentThread->start(updateExperiment, CTHREAD_PRIORITY_GRAPHICS);
    glutTimerFunc(50, graphicsTimer, 0);
    glutMainLoop();



    // close everything
    close();

    // exit
    return 0;




}

	



#endif // TEST_NIDAQ_FT