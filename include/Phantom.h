
#ifndef PHANTOM_H
#define PHANTOM_H

#include "chai3d.h"
#include "shared_Data.h"

void initPhantom(void);
void linkSharedDataToPhantom(shared_data& sharedData);
void updatePhantom(void);
void closePhantom(void);


#endif  // PHANTOM_H
