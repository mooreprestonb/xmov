/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

#include "glsetvalues.h++"
#include "typedefs.h"

void display(SETVALUES &,SYSTEM &,DLIST &);
void reshape(SETVALUES &,int w,int h);
void gen_display_list(SYSTEM &,DLIST &dlist,SETVALUES &);

void display_cell(CELLS cell,SETVALUES &setvalues,JSCELLS *jscell);
#endif
