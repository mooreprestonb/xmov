
#include "util_math.h"
#include "printinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**************************************************************/
#ifndef MAX
#define MAX(A,B) ((A>B)?(A):(B))
#endif
#ifndef MIN
#define MIN(A,B) ((A<B)?(A):(B))
#endif

const int DIM=3;

/************************************************************************************************/

float printdist(int a1,int a2,float xn[])
{
  int i;
  float d[DIM],dist;

  for(i=0;i<DIM;++i) d[i] = xn[a1*DIM+i]-xn[a2*DIM+i]; 
  dist = sqrt(dot(DIM,d,d)); 
  printf("distance %d-%d = %g Ang\n",a1,a2,dist);
  return dist;
}

/************************************************************************************************/

float printangle(int a1,int a2,int a3, float xn[])
{
  int i;
  float d1[DIM],d2[DIM],ang,dist1,dist2;

  for(i=0;i<DIM;++i) d1[i] = xn[a1*DIM+i]-xn[a2*DIM+i]; 
  dist1 = norm(DIM,d1);
  
  for(i=0;i<DIM;++i) d2[i] = xn[a3*DIM+i]-xn[a2*DIM+i]; 
  dist2 = norm(DIM,d2);

  if(dist1 == 0.0 || dist2 == 0.0){
    ang = 0.;
  } else {
    ang = dot(DIM,d1,d2);
    ang = MAX(ang,-1.);  ang = MIN(ang,1.);  ang = acos(ang);
  }
  printf("Angle %d-%d-%d = %g Deg\n",a1,a2,a3,ang*180./M_PI);
  return ang;
}

/************************************************************************************************/

float printtorsion(int a1,int a2,int a3, int a4, float xn[])
{
  int i;
  float v1[DIM],v2[DIM],v3[DIM]; // bonds
  float nv1[DIM],nv2[DIM]; // norms to planes
  float dn1,dn2,ang;

  for(i=0;i<DIM;++i) v1[i] = xn[a1*DIM+i]-xn[a2*DIM+i]; 
  for(i=0;i<DIM;++i) v2[i] = xn[a2*DIM+i]-xn[a3*DIM+i]; 
  for(i=0;i<DIM;++i) v3[i] = xn[a3*DIM+i]-xn[a4*DIM+i]; 
  
  cross33(v1,v2,nv1); /*The normal to plane1*/
  cross33(v2,v3,nv2); /*The normal to plane2 (keep #2 constant) */

  dn1 = norm(DIM,nv1); // normalize
  dn2 = norm(DIM,nv2);

  if(dn1 == 0 || dn2 ==0){
    ang = 0.0;
  } else {
    ang = dot(DIM,nv1,nv2);
    ang = MAX(ang,-1.);  ang = MIN(ang,1.);  ang = acos(ang);
  }
  printf("Torsional Angle %d-%d-%d-%d = %g Deg\n",a1,a2,a3,a4,ang*180/M_PI);
  return ang;
}
