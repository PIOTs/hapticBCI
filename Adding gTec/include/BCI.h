
#ifndef BCI_H
#define BCI_H

// library inclusion to work with oscpack library
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

#include <string>
#include <map>
#include "UdpSocket.h"
#include "SockStream.h"
#include "OSC_Listener.h"
#include "chai3d.h"
#include "shared_Data.h"

void linkSharedDataToBCI(shared_data& sharedData);
void initBCI(void);
void updateBCI(void);
bool readFromGTec(map<string, float> &state, sockstream &recStream);

#endif  // BCI_H
