#ifndef _cruncher_h_
#define _cruncher_h_

#include "bb.h"
#include "file.h"

bool crunch(File *aSource, File *aTarget, uint startAdress, uint decrFlag, bool isRelocated);

#endif // _cruncher_h_
