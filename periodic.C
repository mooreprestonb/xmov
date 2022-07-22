/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "glsetvalues.h++"
#include "typedefs.h"

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

// #define DEBUG

#ifndef anint
#define anint(A) ((int)((A)+((A)>=0?.5:-.5)))
#endif

/* ------------  now make the periodic boundaries ---------  */
void periodic(SYSTEM &system,SETVALUES &sv)
{
  int np,i,j,k,l,napm,iframe,natoms,ii;
  float cm[3],cn[3],sx[3],tm,hmati[9],hmat[9];
  float *px;
  int iperd=3;
 
  iframe = sv.frame;
  int patoms = sv.iperda;
  natoms = system.natoms;
  px = system.coords[iframe].getx();

#ifdef DEBUG
  printf("Number of atoms in periodic_boundaries = %d (frame = %d\n",
	 natoms,iframe);
#endif

  for(i=0;i<9;i++) hmat[i] = system.cell[iframe][i];
  /* get inverse matrix */
  tm = 1./(hmat[0]*(hmat[4]*hmat[8] - hmat[5]*hmat[7]) + 
	   hmat[1]*(hmat[5]*hmat[6] - hmat[3]*hmat[8]) + 
	   hmat[2]*(hmat[3]*hmat[7] - hmat[4]*hmat[6]));
  
  hmati[0] = (hmat[4]*hmat[8] - hmat[5]*hmat[7])*tm;
  hmati[1] = (hmat[2]*hmat[7] - hmat[1]*hmat[8])*tm;
  hmati[2] = (hmat[1]*hmat[5] - hmat[2]*hmat[4])*tm;
  hmati[3] = (hmat[5]*hmat[6] - hmat[3]*hmat[8])*tm;
  hmati[4] = (hmat[0]*hmat[8] - hmat[2]*hmat[6])*tm;
  hmati[5] = (hmat[2]*hmat[3] - hmat[0]*hmat[5])*tm;
  hmati[6] = (hmat[3]*hmat[7] - hmat[4]*hmat[6])*tm;
  hmati[7] = (hmat[1]*hmat[6] - hmat[0]*hmat[7])*tm;
  hmati[8] = (hmat[0]*hmat[4] - hmat[1]*hmat[3])*tm;

#ifdef DEBUG
  printf("hmata:%g %g %g\n",hmat[0],hmat[1],hmat[2]);
  printf("hmatb:%g %g %g\n",hmat[3],hmat[4],hmat[5]);
  printf("hmatc:%g %g %g\n",hmat[6],hmat[7],hmat[8]);
  printf("hmatai:%g %g %g\n",hmati[0],hmati[1],hmati[2]);
  printf("hmatbi:%g %g %g\n",hmati[3],hmati[4],hmati[5]);
  printf("hmatci:%g %g %g\n",hmati[6],hmati[7],hmati[8]);
#endif

  np = 0;
  while(np<natoms){
    ii = np*DIM;
    napm = 0;

    if(patoms) {
      napm = 1;
    } else {
      i = system.atoms[np].ispec();
      j = system.atoms[np].imole();
      while(system.atoms[np+napm].ispec()==i && 
	    system.atoms[np+napm].imole()==j)
	napm++;
    }
    
#ifdef DEBUG
    printf("atom number = %d species = %d napm = %d\n",np,j,napm);
#endif
    for(k=0;k<DIM;k++) cm[k] = cn[k] = sx[k] = 0.;
    for(tm=0.,j=0;j<napm;j++){
      for(k=0;k<DIM;k++) cm[k] += px[ii+j*DIM+k]*system.atoms[np+j].amass();
      tm += system.atoms[np+j].amass();
    }
    for(k=0;k<DIM;k++) cm[k] /= tm;
    
#ifdef DEBUG
    printf("Center of mass before = %g %g %g tm=%g\n",cm[0],cm[1],cm[2],tm);
#endif

    /* find out how many boxs to subtract */
    for(k=0;k<DIM;k++) for(l=0;l<DIM;l++) sx[k]+=cm[l]*hmati[k*DIM+l];
    for(k=0;k<DIM;k++) if(iperd > k) sx[k] -= anint(sx[k]);
    for(k=0;k<DIM;k++) for(l=0;l<DIM;l++) cn[k]+=sx[l]*hmat[k*DIM+l];
    
#ifdef DEBUG
    printf("Center of mass after = %g %g %g\n",cn[0],cn[1],cn[2]);
#endif
    for(k=0;k<DIM;k++) cm[k] -= cn[k];
    for(j=0;j<napm;j++) for(k=0;k<DIM;k++) px[ii+j*DIM+k] -= cm[k];
    np += napm;
  }
}

/*-------------------------------------------------------------------*/
