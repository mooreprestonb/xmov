#include <Xm/MainW.h>
#include <Xm/ScrolledW.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "menuutil.h"
#include "display.h"
#include "lighting.h"
#include "read_things.h"
#include "event_func.h"
#include "util_math.h"
#include "trackball.h"

#define TRACKBALLSIZE  (0.8)
#define RENORMCOUNT 97

extern Widget da;

extern SYSTEM sys;
extern SETVALUES sv;

static void axis_quat(float a[3], float phi, float q[4]);
extern void redraw(void);

Boolean rotate(XtPointer client_data)
/********************************************************************/
{
  float a[3],phi;
  char *axis = (char *) client_data;
  
  phi = -2.*M_PI/180.;
  if (axis[1]=='m' ) phi = -phi;
  
  switch ( *axis ) {
  case 'x':
    a[0] = 1.;  a[1] = 0.;  a[2] = 0.;
    axis_quat(a,phi,sv.aquat);
    break;
  case 'y':
    a[0] = 0.;  a[1] = 1.;  a[2] = 0.;
    axis_quat(a,phi,sv.aquat);
    break;
  case 'z':
    a[0] = 0.;  a[1] = 0.;  a[2] = 1.;
    axis_quat(a,phi,sv.aquat);
    break;
  default:
    break;
  }

  add_quats(sv.aquat,sv.cquat,sv.cquat);  
  build_rotmat(sv.rot_mat,sv.cquat);

  /*
  int i;
  for ( i=0; i<16; i++ ) rmat[i]=0;
  rmat[15]=1;
  if ( sv.rot_type==0 ) {
    mmat16ba(sv.rot_mat,rmat);
  } else {
    mmat16ab(sv.rot_mat,rmat);
  }
  */
  redraw();
  
  return False;
}

void euler(void)
/********************************************************************/
{
   float theta,phi,psi;
   float sd1,d1,sd2,d2;

   d1=fabs(sd1=sv.rot_mat[10]);
   theta=acos(d1>1? sd1/d1: sd1);
   if ( theta==0 || theta==M_PI ) {
      phi=atan2(sv.rot_mat[1],sv.rot_mat[0]);
      psi=0;
   } else {
      phi=atan2(sv.rot_mat[8],-sv.rot_mat[9]);
      d2=fabs(sd2=cos(phi)*sv.rot_mat[0]+sin(phi)*sv.rot_mat[1]);   
      psi=acos(d2>1? sd2/d2: sd2);
      if ( sv.rot_mat[2]<0 ) psi=2*M_PI-psi;
   }
   if ( phi<0 ) phi += 2*M_PI;
   
   printf("phi %f   theta %f   psi %f\n",phi*180/M_PI,theta*180/M_PI,
      psi*180/M_PI);
}


/********************************************************************/
void euler_rot(void)
{
   float theta,phi,psi,c,s,rmat[16];
   int i;

   printf("Input phi:\n");
   scanf("%f",&phi);
   printf("Input theta:\n");
   scanf("%f",&theta);
   printf("Input psi:\n");
   scanf("%f",&psi);

   printf("phi=%f, theta=%f, psi=%f\n",phi,theta,psi);

   for ( i=0; i<16; i++ ) sv.rot_mat[i]=0;
   sv.rot_mat[0]=1;
   sv.rot_mat[5]=1;
   sv.rot_mat[10]=1;
   sv.rot_mat[15]=1;

   for ( i=0; i<16; i++ ) rmat[i]=0;
   rmat[15]=1;

   c=cos(psi*M_PI/180);
   s=sin(psi*M_PI/180);

   rmat[0] = c;   rmat[1] = s;
   rmat[4] =-s;   rmat[5] = c;
                                rmat[10] = 1;

   mmat16ba(sv.rot_mat,rmat);

   for ( i=0; i<16; i++ ) rmat[i]=0;
   rmat[15]=1;

   c=cos(theta*M_PI/180);
   s=sin(theta*M_PI/180);

   rmat[0] = 1;
                 rmat[5] = c;   rmat[6] = s;
                 rmat[9] =-s;   rmat[10]= c;

   mmat16ba(sv.rot_mat,rmat);

   for ( i=0; i<16; i++ ) rmat[i]=0;
   rmat[15]=1;

   c=cos(phi*M_PI/180);
   s=sin(phi*M_PI/180);

   rmat[0] = c;   rmat[1] = s;
   rmat[4] =-s;   rmat[5] = c;
                                rmat[10] = 1;

   mmat16ba(sv.rot_mat,rmat);

   redraw();
}

static float p_sphere(float, float, float);
/* static void normalize_quat(float[4]); */

void vset(float *v, float x, float y, float z){v[0]=x;v[1]=y;v[2]=z;}
void vcopy(const float *v1, float *v2)
{
  int i;
  for (i = 0; i < 3; i++) v2[i] = v1[i];
}

void vcross(float *v1,float *v2,float *cross)
{
  float t1,t2,t3;

  t1 = (v1[1] * v2[2]) - (v1[2] * v2[1]);
  t2 = (v1[2] * v2[0]) - (v1[0] * v2[2]);
  t3 = (v1[0] * v2[1]) - (v1[1] * v2[0]);
  cross[0] = t1;
  cross[1] = t2;
  cross[2] = t3;
}

void vscale(float *v, float div){v[0] *= div; v[1] *= div; v[2] *= div;}
float vdot(float *v1,float *v2){return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2];}
float vlength(float *v){return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);}

void vadd(float *src1,float *src2,float *dst)
{
  dst[0] = src1[0] + src2[0];
  dst[1] = src1[1] + src2[1];
  dst[2] = src1[2] + src2[2];
}

void trackball(float q[4], float p1x, float p1y, float p2x, float p2y)
{
  float a[3],p1[3], p2[3], d[3];
  float phi,t,p1z,p2z;

  /* Zero rotation */
  if (p1x == p2x && p1y == p2y) {
    q[0] = q[1] = q[2] = 0.0;
    q[3] = 1.0;
    return;
  }
  /* figure out z-coordinate */
  p1z =  p_sphere(TRACKBALLSIZE, p1x, p1y);
  p2z =  p_sphere(TRACKBALLSIZE, p2x, p2y);
  vset(p1, p1x, p1y, p1z);
  vset(p2, p2x, p2y, p2z);

  /* axis of rotation*/
  vcross(p2, p1, a);

  /* Figure out how much to rotate around that axis. */
  d[0] = p1[0] - p2[0];
  d[1] = p1[1] - p2[1];
  d[2] = p1[2] - p2[2];

  t = vlength(d)/(2.*TRACKBALLSIZE);

  /* Avoid problems with out-of-control values. */
  if (t > 1.0)  t = 1.0;
  if (t < -1.0) t = -1.0;
  phi = 2.0 * asin(t);

  axis_quat(a, phi, q);
}

/* Given an axis and angle, compute quaternion. */
void axis_quat(float a[3], float phi, float q[4])
{
  int i;
  float mag;

  mag = sin(phi/2.)/sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);
  for(i=0;i<3;i++) q[i] = a[i]*mag;
  q[3] = cos(phi / 2.0);
}

/* Project an x,y pair onto a sphere of radius r OR a
   hyperbolic sheet if we are away from the center of the
   sphere. */
static float p_sphere(float r, float x, float y)
{
  float d, z;

  d = sqrt(x*x+y*y);
  if (d<r*M_SQRT1_2) z= sqrt(r*r-d*d);  /* Inside sphere. */
  else z = r*r/(2.*d);              /* On hyperbola. */

  return z;
}

void add_quats(float q1[4], float q2[4], float d[4])
{
  int i;
  static int count = 0;
  float t1[4],t2[4],t3[4],tf[4],mag;

  vcopy(q1, t1);
  vscale(t1, q2[3]);

  vcopy(q2, t2);
  vscale(t2, q1[3]);

  vcross(q2, q1, t3);
  vadd(t1, t2, tf);
  vadd(t3, tf, tf);
  tf[3] = q1[3] * q2[3] - vdot(q1, q2);

  d[0] = tf[0];
  d[1] = tf[1];
  d[2] = tf[2];
  d[3] = tf[3];

  if (++count > RENORMCOUNT) {
    count = 0;
    mag=sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]+d[3]*d[3]);
    for (i=0;i<4;i++) d[i] /= mag;
  }
}

void build_rotmat(GLfloat m[16], float q[4])
{
  m[0] =1.0-2.0*(q[1]*q[1]+q[2]*q[2]);
  m[1] =2.0*(q[0]*q[1]-q[2]*q[3]);
  m[2] =2.0*(q[2]*q[0]+q[1]*q[3]);
  m[3] =0.0;
  
  m[4] =2.0*(q[0]*q[1]+q[2]*q[3]);
  m[5] =1.0-2.0*(q[2]*q[2]+q[0]*q[0]);
  m[6] =2.0*(q[1]*q[2]-q[0]*q[3]);
  m[7] =0.0;
  
  m[8] =2.0*(q[2]*q[0]-q[1]*q[3]);
  m[9] =2.0*(q[1]*q[2]+q[0]*q[3]);
  m[10]=1.0-2.0*(q[1]*q[1]+q[0]*q[0]);
  m[11]=0.0;

  m[12]=0.0;
  m[13]=0.0;
  m[14]=0.0;
  m[15]=1.0;
}
