/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include "glsetvalues.h++"
#include "typedefs.h"

#ifndef INIT_VALUES_H_INCLUDED
#define INIT_VALUES_H_INCLUDED

void initSys(SYSTEM &system);
void initSetValues(int argc,char *argv[],SETVALUES &sv);
void initGL(SETVALUES &sv);

#endif
