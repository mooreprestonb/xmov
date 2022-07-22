/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

/* #define CELL0 */

#include "display.h"
#include "lighting.h"
#include "find_things.h"

typedef struct {
  GLfloat x, y;
} jitter_point;

#define ACSIZE  8
static jitter_point jit[ACSIZE] =
{
  {-0.334818,  0.435331},  { 0.286438, -0.393495},
  { 0.459462,  0.141540},  {-0.414498, -0.192829},
  {-0.183790,  0.082102},  {-0.079263, -0.317383},
  { 0.102254,  0.299133},  { 0.164216, -0.054399}
};

/*-----------------------------------------------------------------*/
/*  accPerspective()
 * 
 *  The first 4 arguments are identical to the gluPerspective() call.
 *  pixdx and pixdy are anti-alias jitter in pixels. 
 *  Set both equal to 0.0 for no anti-alias jitter.
 *  eyedx and eyedy are depth-of field jitter in pixels. 
 *  Set both equal to 0.0 for no depth of field effects.
 *
 *  focus is distance from eye to plane in focus. 
 *  focus must be greater than, but not equal to 0.0.
 *
 */
static void accPerspective(GLfloat fovy, GLfloat aspect, 
   GLfloat near, GLfloat far, GLfloat pixdx, GLfloat pixdy, 
   GLfloat eyedx, GLfloat eyedy, GLfloat focus)
/*************************************************************************/
{
  GLint viewport[4];
  GLfloat xwsize, ywsize,dx, dy;
  GLfloat fov2,left,right,bottom,top;
  
  fov2 = ((fovy*M_PI) / 180.0) / 2.0;
  
  top = near / (cos(fov2) / sin(fov2));
  bottom = -top;
  right = top * aspect;
  left = -right;  
  xwsize = right - left;
  ywsize = top - bottom;

  glGetIntegerv (GL_VIEWPORT, viewport);
    
  dx = -(pixdx*xwsize/(GLfloat) viewport[2] + eyedx*near/focus);
  dy = -(pixdy*ywsize/(GLfloat) viewport[3] + eyedy*near/focus);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum (left + dx, right + dx, bottom + dy, top + dy, near, far); 
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef (-eyedx, -eyedy, 0.0);
}
/*-----------------------------------------------------------------*/

void anti_display(SETVALUES &sv,SYSTEM *system,DLIST *dlist)
{
  int f,jitter;
  GLint viewport[4];
  GLfloat bits[2];

  f = sv.frame % system->nconf;

  glDrawBuffer(GL_FRONT);
  glGetFloatv(GL_ACCUM_ALPHA_BITS,bits);
  glReadBuffer(GL_FRONT);
  glClearAccum(0,0,0,0);
  glGetIntegerv (GL_VIEWPORT, viewport);
  glClear(GL_ACCUM_BUFFER_BIT);
  for ( jitter=0; jitter<ACSIZE; jitter++ ) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    accPerspective (40.0,(GLfloat) viewport[2]/(GLfloat) viewport[3], 
		    1., 10., jit[jitter].x, jit[jitter].y,0., 0., 1.);
    glTranslatef(0.,0.,-3.);
    glPushMatrix();
    glTranslatef(sv.transx,sv.transy,sv.transz);
    glMultMatrixf(sv.rot_mat);
    glScalef(sv.scalefactor,sv.scalefactor,sv.scalefactor);
    /* display screen */
    if( sv.iatom) displayAtoms(f,system->natoms,system->atoms,system->coords,sv);
    if( sv.bonds ) {
      int nb = system->bonds[f].nbonds;
      BOND *bond = (system->bonds[f]).bond;
      float *x = (system->coords[f]).getx();
      displayBonds(nb,bond,x,system->atoms,sv);
    }
    if(sv.icell) display_cell(system->cell[f],sv,system->jscell);
    if(sv.hydro) displayHydro(f,*system,sv);
    if(sv.vectors) {
      int iv = (f>=system->nvectors)? system->nvectors - 1 : f;
      displayVectors(system->vectors[iv],sv);
    }
    glPopMatrix();
    glAccum(GL_ACCUM, 1.0/ACSIZE);
  }
  glAccum (GL_RETURN, 1.0);
  glFlush();
  printf("Antialiasing complete sleep for 6 seconds\n");
  sleep(6);
  glDrawBuffer(GL_BACK);
  glReadBuffer(GL_BACK);
}




