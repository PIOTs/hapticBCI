
#ifndef EMOTIV_H
#define EMOTIV_H

// library inclusion to work with oscpack library
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include "UdpSocket.h"
#include "OSC_Listener.h"
#include "chai3d.h"
#include "shared_Data.h"

void linkSharedDataToEmotiv(shared_data& sharedData);
void initEmotiv(void);
void updateEmotiv(void);

#endif  // EMOTIV_H
