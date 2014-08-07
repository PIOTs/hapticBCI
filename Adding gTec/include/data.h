
#ifndef DATA_H
#define DATA_H

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "chai3d.h"
#include "UdpSocket.h"
#include "cNeuroTouch.h"
#include "experiment.h"
#include "shared_Data.h"

void linkSharedData(shared_data& sharedData);
void setup(void);
void saveOneTimeStep(void);
void recordTrial(void);

#endif  // DATA_H
