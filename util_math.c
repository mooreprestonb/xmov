/* math routines that might be useful */

#include <stdio.h>
#include <math.h>
#include "util_math.h"

/**************************************************************/
#ifndef MAX
#define MAX(A,B) ((A>B)?(A):(B))
#endif
#ifndef MIN
#define MIN(A,B) ((A<B)?(A):(B))
#endif


/**************************************************************/
float getinv9(float mat[], float mati[])
{
  float rvol;

  /* gets inverse, mati, of the iperd dimensional matrix mat */
  rvol = 1./(mat[0]*(mat[4]*mat[8] - mat[5]*mat[7]) + 
	     mat[1]*(mat[5]*mat[6] - mat[3]*mat[8]) + 
	     mat[2]*(mat[3]*mat[7] - mat[4]*mat[6]));
  
  mati[0] = (mat[4]*mat[8] - mat[5]*mat[7])*rvol;
  mati[1] = (mat[2]*mat[7] - mat[1]*mat[8])*rvol;
  mati[2] = (mat[1]*mat[5] - mat[2]*mat[4])*rvol;
  mati[3] = (mat[5]*mat[6] - mat[3]*mat[8])*rvol;
  mati[4] = (mat[0]*mat[8] - mat[2]*mat[6])*rvol;
  mati[5] = (mat[2]*mat[3] - mat[0]*mat[5])*rvol;
  mati[6] = (mat[3]*mat[7] - mat[4]*mat[6])*rvol;
  mati[7] = (mat[1]*mat[6] - mat[0]*mat[7])*rvol;
  mati[8] = (mat[0]*mat[4] - mat[1]*mat[3])*rvol;

  return (1./rvol);
}

/*-------------------------------------------------------------------------*/
float get_det(float *mat)
{
  /* gets det of mat */
  
  return (mat[0]*(mat[4]*mat[8] - mat[5]*mat[7]) + 
	  mat[1]*(mat[5]*mat[6] - mat[3]*mat[8]) + 
	  mat[2]*(mat[3]*mat[7] - mat[4]*mat[6]));
} 
/*-------------------------------------------------------------------------*/
void sym33(float mat[9])
{
  float tmp;

  tmp = .5*(mat[1] + mat[3]);
  mat[1] = mat[3] = tmp;
  tmp = .5*(mat[2] + mat[6]);
  mat[2] = mat[6] = tmp;
  tmp = .5*(mat[5] + mat[7]); 
  mat[5] = mat[7] = tmp;
}
/*-------------------------------------------------------------------------*/

float ddot1a(int n,float *a,int astep,float *b,int bstep)
{
  int i,j,k;
  float ddot1;

  ddot1 = 0.;
  for(i=0,j=0,k=0; i<n ;i++,j+=astep,k+=bstep){
    ddot1 += a[j]*b[k];
  }

  return ddot1;
}

/*-------------------------------------------------------------------------*/
float ddot1(int n,float *a,float *b)
{
  int i;
  float ddot1;

  ddot1 = 0.;
  for(i=0; i<n ;i++) ddot1 += a[i]*b[i];

  return ddot1;
}

/*-------------------------------------------------------------------------*/

float dsum1a(int n,float *a,int astep)
{
  int i,j;
  float dsum1;

  dsum1 = 0.;
  for(i=0,j=0;i<n;i++,j+=astep){
    dsum1 += a[j];
  }

  return dsum1;
}
/*-------------------------------------------------------------------------*/

float dsum1(int n,float *a)
{
  int i;
  float dsum1;

  dsum1 = 0.;
  for(i=0; i<n ;i++) dsum1 += a[i];

  return dsum1;
}

/**************************************************************/
void mmat16ba(float *a,float *b)
{
  int i,j,k;
  float c[16],sum;
  
  for(i=0;i<16;i++) c[i] = 0.0;

  for(i=0;i<4;i++){
    for(j=0;j<4;j++){
      sum = 0.;
      for(k=0;k<4;k++)sum += b[i + 4*k] * a[k + 4*j];
      c[i + 4*j] = sum;
    }
  }
  for(i=0;i<16;i++) a[i] = c[i];
}

/**************************************************************/
void mmat16ab(float *a,float *b)
{
  int i,j,k;
  float c[16],sum;
  
  for(i=0;i<16;i++) c[i] = 0.0;
  
  for(i=0;i<4;i++){
    for(j=0;j<4;j++){
      sum = 0.;
      for(k=0;k<4;k++)sum += a[i + 4*k] * b[k + 4*j];
      c[i + 4*j] = sum;
    }
  }
  for(i=0;i<16;i++) a[i] = c[i];
}

/**************************************************************/
void cross(float vx,float vy,float vz,float wx,float wy,float wz,
   float *tx,float *ty,float *tz)
{
  *tx = vy*wz-wy*vz;  *ty = vz*wx-wz*vx;  *tz = vx*wy-wx*vy;
}
/**************************************************************/
float length(float x,float y,float z)
{
  return sqrt(x*x+y*y+z*z);
}
/**************************************************************/
void get_normal(float x1,float y1,float z1,float x2,float y2,float z2,
   float x3,float y3,float z3,float *xn,float *yn,float *zn)
{
  float vx,vy,vz,wx,wy,wz,rl;

  vx = x2-x1;  vy = y2-y1;  vz = z2-z1;
  wx = x2-x3;  wy = y2-y3;  wz = z2-z3;
  cross(vx,vy,vz,wx,wy,wz,xn,yn,zn);
  rl = length(*xn,*yn,*zn);
  if(rl != 0) {
    *xn /= rl;    *yn /= rl;    *zn /= rl;
  } else {
    fprintf(stderr,"Normal has zero lenght!!! - Set to z direction\n");
    *xn = 0.; *yn = 0.; *zn = 1.;
  }
}

  
/************************************************************************************************/
float dot(int n,float a[],float b[])
{
  int i;
  float dp = 0.0;
  for(i=0;i<n;++i) dp += a[i]*b[i];
  return dp;
}

/************************************************************************************************/
void scalevec(const int n,const float a,float b[])
{
  int i;
  for(i=0;i<n;++i) b[i] *= a;
}

/************************************************************************************************/
void addvec(int n,float a,float b[])
{
  int i;
  for(i=0;i<n;++i) b[i] += a;
}

/************************************************************************************************/
void addvecs(int n,float a[],float b[], float c[])
{
  int i;
  for(i=0;i<n;++i) c[i] = a[i]+b[i];
}

/************************************************************************************************/
void cross33(float a[3],float b[3],float c[3])
{
  c[0] = a[1]*b[2] - a[2]*b[1];
  c[1] = a[2]*b[0] - a[0]*b[2];
  c[2] = a[0]*b[1] - a[1]*b[0];
}

/************************************************************************************************/
float norm(int n,float a[])
{
  int i;
  float dn = sqrt(dot(n,a,a));
  if(dn == 0) return 0.;
  for(i=0;i<n;++i) a[i] /= dn;
  return dn;
}

