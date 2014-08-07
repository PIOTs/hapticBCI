//===========================================================================
/*
    This file is based on the CHAI 3D visualization and haptics libraries,
	but not a standard part of it.

    This is meant to contain function calls that will be used with the Haptic
	Shear Stylus developed in Dr. Okamura's CHARM lab at Stanford University
	with the guidance of Dr. Will Provancher

    \author    Robert Romano
*/
//===========================================================================
#include "shared_Data.h"
//---------------------------------------------------------------------------
#ifndef CShearStylusH
#define CShearStylusH
//---------------------------------------------------------------------------
//#include "devices/CGenericHapticDevice.h"

extern "C"
{
#include "Win626.h"
}

#include "windows.h"
#include "APP626.h"
#include <stdlib.h>
//---------------------------------------------------------------------------

//===========================================================================
/*!
    \file       cNeuroTouch.h

    \brief
    <b> Devices </b> \n 
    Haptic Shear Stylus.
*/
//===========================================================================

// Set of defines used for encoder functionality with the Sensoray 626
#define LOADSRC_INDX 0 // Preload core in response to Index.
#define LOADSRC_OVER 1 // Preload core in response to Overflow.
#define LOADSRCB_OVERA 2 // Preload B core in response to A Overflow.
#define LOADSRC_NONE 3 // Never preload core.
// IntSrc values:
#define INTSRC_NONE 0 // Interrupts disabled.
#define INTSRC_OVER 1 // Interrupt on Overflow.
#define INTSRC_INDX 2 // Interrupt on Index.
#define INTSRC_BOTH 3 // Interrupt on Index or Overflow.
// LatchSrc values:
#define LATCHSRC_AB_READ 0 // Latch on read.
#define LATCHSRC_A_INDXA 1 // Latch A on A Index.
#define LATCHSRC_B_INDXB 2 // Latch B on B Index.
#define LATCHSRC_B_OVERA 3 // Latch B on A Overflow.
// IndxSrc values:
#define INDXSRC_HARD 0 // Hardware or software index.
#define INDXSRC_SOFT 1 // Software index only.
// IndxPol values:
#define INDXPOL_POS 0 // Index input is active high.
#define INDXPOL_NEG 1 // Index input is active low.
// ClkSrc values:
#define CLKSRC_COUNTER 0 // Counter mode.
#define CLKSRC_TIMER 2 // Timer mode.
#define CLKSRC_EXTENDER 3 // Extender mode.
// ClkPol values:
#define CLKPOL_POS 0 // Counter/Extender clock is active high.
#define CLKPOL_NEG 1 // Counter/Extender clock is active low.
#define CNTDIR_UP 0 // Timer counts up.
#define CNTDIR_DOWN 1 // Timer counts down.
// ClkEnab values:
#define CLKENAB_ALWAYS 0 // Clock always enabled.
#define CLKENAB_INDEX 1 // Clock is enabled by index.
// ClkMult values:
#define CLKMULT_4X 0 // 4x clock multiplier.
#define CLKMULT_2X 1 // 2x clock multiplier.
#define CLKMULT_1X 2 // 1x clock multiplier.
// Bit Field positions in COUNTER_SETUP word:
#define BF_LOADSRC 9 // Preload trigger.
#define BF_INDXSRC 7 // Index source.
#define BF_INDXPOL 6 // Index polarity.
#define BF_CLKSRC 4 // Clock source.
#define BF_CLKPOL 3 // Clock polarity/count direction.
#define BF_CLKMULT 1 // Clock multiplier.
#define BF_CLKENAB 0 // Clock enable.
// Counter channel numbers:
// Sensoray Instruction Manual 39 Model 626 Driver for Windows
#define CNTR_0A 0 // Counter 0A.v
#define CNTR_1A 1 // Counter 1A.
#define CNTR_2A 2 // Counter 2A.
#define CNTR_0B 3 // Counter 0B.
#define CNTR_1B 4 // Counter 1B.
#define CNTR_2B 5 // Counter 2B.

// Define used for Sensoray Encoder
#define MAX_COUNT 16777215

// Define used for DAC Voltage Conversion
#define DAC_VSCALAR 819.1

// Defines for DAC channels
#define DAC0 0 // Pin 42 on J1
#define DAC1 1 // Pin 44 on J1 ( Motor A)
#define DAC2 2 // Pin 46 on J1 ( Motor B)
#define DAC3 3 // Pin 48 on J1

// Defines for ADC
#define RANGE_10V	0x00 // Range code for ADC +/- 10V range
#define RANGE_5V	0x10 // Range code for ADC +/- 5V range
#define EOPL		0x80 // ADC end-of-poll-list marker
#define CHANMASK	0x0F // ADC channel number mask
#define MAX_AD_VALUE 32767

// Encode count to rad/deg conversions
#define PI 3.1415926535897932384
//#define ENCCOUNT_TO_RAD 2*3.1415926535897932384/((1539.0/65)*512.0*4.0)
#define ENCCOUNT_TO_RAD 2*PI/((1/1)*500.0*4.0) // 2pi/( (Gear Ratio)*ClocksPerRevolution*QuadratureEncoding )
#define ENCCOUNT_TO_DEG 360/((1/1)*512.0*4.0)				   // 360deg/( (Gear Ratio)*ClocksPerRevolution*QuadratureEncoding )




//===========================================================================
/*!
    \class      cNeuroTouch
    \ingroup    devices  

    \brief  
    cNeuroTouch describes an interface to the custom built Shear Stylus
		device developed in Dr. Allison Okamura's CHARM Lab at Stanford
		University
*/
//===========================================================================
class cNeuroTouch
{
  public:

    //-----------------------------------------------------------------------
    // CONSTRUCTOR & DESTRUCTOR:
    //-----------------------------------------------------------------------

    //! Constructor of cNeuroTouch.
    cNeuroTouch(unsigned int a_deviceNumber = 0);

    //! Destructor of cNeuroTouch.
    virtual ~cNeuroTouch();

    //-----------------------------------------------------------------------
    // METHODS:
    //-----------------------------------------------------------------------

    // Open connection to haptic device (0 indicates success).
    int open();

    // Close connection to haptic device (0 indicates success).
    int close();

    // Read the position of the skin stretch device. Units are meters [m].
    int getPosition(float& a_position,float& b_position);

    // Send a force [N] to the skin stretch device.
    int setForce(const double& x_force, const double& y_force);

	// zero the encoders for calibration
	int cNeuroTouch::zeroEncoders(void);

	// initialize ADC
	int cNeuroTouch::initADC(void);

	// Query Function
	int cNeuroTouch::retrieveState(float& TorqueMA, float& TorqueMB, float& TorqueCA, float& TorqueCB, float& PositionMA, float& PositionMB, float& ForceEEx, float& ForceEEy, float& TimeStamp);


	// TEST HARNESS

	//ADC test harness
	int cNeuroTouch::testHarnessADC(void);





  private:

    // Device ID number.
    int m_deviceID;

};

//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
