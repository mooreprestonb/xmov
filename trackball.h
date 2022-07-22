
/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef TRACKBALL_H_INCLUDED

#include "typedefs.h"

extern Boolean rotate(XtPointer client_data);
extern void mmat16ba(float *a,float *b);
extern void mmat16ab(float *a,float *b);
extern void euler(void);
extern void euler_rot(void);
extern void trackball(float [],float,float,float,float);
extern void add_quats(float q1[4], float q2[4], float dest[4]);
extern void build_rotmat(GLfloat m[16], float q[4]);

#define TRACKBALL_H_INCLUDED
#endif



