/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef UTIL_MATH_H_INCLUDED
#define UTIL_MATH_H_INCLUDED

extern float getinv9(float [], float []);
extern void mmat16ab(float *,float *);
extern void mmat16ba(float *,float *);
extern void get_normal(float,float,float,float,float,float,float,float,float,
   float *,float *,float *);
extern float lenght(float x,float y,float z);

float dot(int n,float [],float []);
void scalevec(const int ,const float,float []);
float norm(int n,float []);
void cross33(float[3],float[3],float[3]);
#endif



