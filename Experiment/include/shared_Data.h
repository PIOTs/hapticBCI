
#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include "chai3d.h"
#include "UdpSocket.h"
#include "OSC_Listener.h"
#include "cNeuroTouch.h"
#include "NIDAQcommands.h"
#include "NIDAQmx.h"
#include "cForceSensor.h"
#include "cATIForceSensor.h"
using namespace chai3d;
using namespace std;


// operation mode
#define DEMO       0
#define EXPERIMENT 1
// input mode
#define AUTO    0  // cursor moves on its own
#define BCI     1
#define PHANTOM 2
// brain-computer interfaces
#define EMOTIV    0
#define PORT      7400                   // port that "Mind your OSCs" broadcasts to
#define GTEC      1
#define SEND_SOCK "192.168.1.235:20321"  // "IP address:port" defining UDP socket sending state data from g.MOBIlab+
#define REC_SOCK  "192.168.1.195:20320"  // "IP address:port" defining UDP socket receiving state data into BrainGate desktop
// force sensing
#define FS_CALIB "C:\CalibrationFiles\FT13574.cal"
#define FS_INIT  "Dev1/ai0:5"
// control paradigms
#define HAPTICS_OFF     0
#define POS_WITH_CURSOR 1
#define POS_ANTI_CURSOR 2
#define VEL_WITH_CURSOR 3
#define VEL_ANTI_CURSOR 4
// experiment states
#define START_UP     0
#define NO_HAPTICS_1 1
#define HAPTICS_1    2
#define NO_HAPTICS_2 3
#define HAPTICS_2    4
#define PREBLOCK     5
#define BREAK        6
#define RECORD       7
#define THANKS       8
// experiment parameters (just side)
#define LEFT  -1
#define RIGHT  1
#define TARGET_SIZE 0.04  // target radius
#define TARGET_DIST 0.18  // distance of target from center
#define CURSOR_SIZE 0.02  // cursor radius
// thread timing
#define LOOP_TIME 0.001  // for regulating thread loop rates (sec)


// data to save (NOTE: see below for any comments on meaning of variables)
typedef struct {
    
    // experiment status
    int d_blockNum;
    int d_trialNum;
    bool d_trialSuccess;
    double d_targetSide;
    double d_cursorPos;
    double d_cursorVel;
    double d_timeElapsed;
    
    // device states
    float d_cogRight;      // for Emotiv
    float d_cogLeft;
    float d_cogNeut;
    float d_controlSig;    // for g.MOBIlab+ (NOTE: can only get one of these depending on BCI2000 task being run)
    //float d_SelectStim;
    double d_eeForceDesX;
    double d_eeForceDesY;
    float d_motorAPos;
    float d_motorBPos;
    double d_force[3];     // only if sensing
    
} save_data;

// data to share between all threads
typedef struct {
    
    // simulation state
    bool simulationRunning;
    bool simulationFinished;  // dictated by NeuroTouch device
    
    // device pointers
    cHapticDeviceHandler* p_phantomHandler;  // handler for the PHANTOM
    cGenericHapticDevicePtr p_Phantom;       // pointer to PHANTOM device
    cNeuroTouch* p_NeuroTouch;               // pointer to NeuroTouch device
    
    // device frequency counters
    cFrequencyCounter phantomFreqCounter;     // counter to measure rate reading from PHANTOM device [Hz]
    cFrequencyCounter neurotouchFreqCounter;  // counter to measure rate reading from/commanding to NeuroTouch [Hz]
    
    // PHANTOM state
    double phantomPos;  // just in X [mm]
    double phantomVel;
    
    // OSC processing
    OSC_Listener listener;  // "hears" messages passed through "Mind your OSCs" port
    
    // UDP sockets and TCP streams (for g.MOBIlab+)
    receiving_udpsocket recSocket;
    sockstream recStream;
    sending_udpsocket sendSocket;
    sockstream sendStream;
    
    // BCI state (cognitive powers for Emotiv, map/control signal for g.MOBIlab+)
    float cogRight;
    float cogLeft;
    float cogNeut;
    map<string, float> state;
    float controlSig;  // just in X
    
    // NeuroTouch state
    float motorAPos;  // [deg] (NOTE: will need kinematics to convert to Cartesian ee positions (post-processing))
    float motorBPos;
    
    // control
    int opMode;
    int input;
    int bci;
    int controller;			 // default for safety
	cPrecisionClock* time;   // running time for autonomous cursor control
    double autoFreq;         // frequency of autonomous cursor movement [Hz] (variable with keyboard input)
    double cursorPos;
    double cursorPosOneAgo;
    double cursorVel;
    double cursorVelOneAgo;
    double eeForceDesX;      // desired, not actual, end-effector force [N]
    double eeForceDesY;
    
    // sensing
    bool sensing;
    cForceSensor g_ForceSensor;
    double force[3];
    
    // graphics
    int targetSide;
    string message;
    
    // experiment
    int experimentState;
    int blockNum;            // NOTE: trials/blocks start at 1
    int trialNum;
    cPrecisionClock* timer;
    double timeElapsed;      // [sec]
    bool trialSuccess;
    
    // data storage
    vector<save_data> trialData;  // for one trial of experiment
    FILE* outputFile;              // output file for entire experiment (all blocks/trials)
    
	// timers to regulate thread loop rates
	cPrecisionClock m_phantomLoopTimer;
	cPrecisionClock m_neurotouchLoopTimer;
	cPrecisionClock m_expLoopTimer;

} shared_data;

#endif  // SHARED_DATA_H
