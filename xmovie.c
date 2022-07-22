/* 
   Copyright 2002 Preston Moore 
*/


#include <stdio.h>
#include "xmotif.h"

#include "glsetvalues.h++"
#include "typedefs.h"

SETVALUES sv;
SYSTEM sys;
DLIST dl;
Widget da,toplevel;

XtAppContext app;
XtTranslations trans = NULL;
Display *dpy;
Window win;

#include "initValues.h++"

void createWindows(int argc,char *argv[],SETVALUES &sv);

/********************************************************************/
int main(int argc,char *argv[])
{
  /* read in command line flags */
  initSys(sys);
  initSetValues(argc,argv,sv);
  sys.iskip = sv.skip;

  /* need to be called before XtVaAppInitialize becaue it modifies argc */
  fprintf(stdout,"Creating windows\n"); fflush(stdout);
  createWindows(argc,argv,sv);
  fprintf(stdout,"Entering event loop\n"); fflush(stdout);
  XtAppMainLoop(app);

  return 0;
}

