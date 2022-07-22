/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include <stdlib.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "lighting.h"

#define LIGHT1
void WireBox (GLfloat width, GLfloat height, GLfloat depth);
void draw_box(GLfloat x0, GLfloat x1, GLfloat y0, GLfloat y1,
	GLfloat z0, GLfloat z1, GLenum type);

/*----------------------------------------------------------------*/
void lighting(SETVALUES &sv)
{
  int i;
  GLfloat white[] = {1.0,1.0,1.0,1.0};
  GLfloat red[] = {1.0,0.0,0.0,1.0};
  GLfloat position[] = { 0.0, 1.0, 0.0, 0.0 };
  GLfloat intensity[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat direction[] = { 0.0, 0.0, -1.0};
#ifdef LIGHT1
  GLfloat light1_position[] = {1.0,1.0,1.0,1.0};
  GLfloat light1_direction[] = {-1.0,-1.0,-1.0};
#endif
  
  for(i=0;i<3;i++){
    intensity[i] *= (float)sv.light_intensity/(float)MAX_LIGHT;
    white[i] *= (float)sv.light_intensity/(float)MAX_LIGHT;
  }
  glLightfv(GL_LIGHT0,GL_SPECULAR,intensity);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,white);

#ifdef LIGHT1
  glLightfv(GL_LIGHT1,GL_SPECULAR,intensity);
  glLightfv(GL_LIGHT1,GL_DIFFUSE,red);
#endif  

  glPushMatrix();
#ifdef LIGHT1
  glLightfv(GL_LIGHT1,GL_POSITION,light1_position);
  glLightfv(GL_LIGHT1,GL_SPOT_DIRECTION,light1_direction);
#endif
  glRotatef((GLfloat) sv.light_spin, 1.0, 0.0, 0.0);
  glLightfv(GL_LIGHT0,GL_POSITION,position);
  glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,direction);

#ifdef WIRE
  glTranslatef(position[0], position[1], position[2]);
  glDisable(GL_LIGHTING);
  glColor3f(1.0, 1.0, 1.0);
  WireBox(0.05,0.05,0.1);
  glEnable(GL_LIGHTING);
#endif

  glPopMatrix();
}
/*----------------------------------------------------------------*/
void light_init(void)
{
  GLfloat lmodel_ambient[] = {.2,.2,.2,1.0};

  glClearColor(0.0,0.0,0.0,0.0);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

#ifdef LIGHT1
  glEnable(GL_LIGHT1);
#endif

  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);

  glLightModelfv(GL_LIGHT_MODEL_AMBIENT,lmodel_ambient);
  glLightfv(GL_LIGHT0,GL_AMBIENT,lmodel_ambient);
#ifdef LIGHT1
  glLightfv(GL_LIGHT1,GL_AMBIENT,lmodel_ambient);
#endif
}

/*----------------------------------------------------------------*/
void set_bcolor(GLfloat bcol[4])
{
  glClearColor(bcol[0],bcol[1],bcol[2],bcol[3]);
}

void WireBox (GLfloat width, GLfloat height, GLfloat depth)
{
  draw_box(-width/2., width/2., -height/2., height/2., 
	   -depth/2., depth/2., GL_LINE_LOOP);
}
/*---------------------------------------------------------*/
/* draw_box:
 *
 * draws a rectangular box with the given x, y, and z ranges.  
 * The box is axis-aligned.
 */
void draw_box(GLfloat x0, GLfloat x1, GLfloat y0, GLfloat y1,
	GLfloat z0, GLfloat z1, GLenum type)
{
  static GLfloat n[6][3] = {
    {-1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {1.0, 0.0, 0.0},
    {0.0, -1.0, 0.0}, {0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}
  };
  static GLint faces[6][4] = {
    { 0, 1, 2, 3 }, { 3, 2, 6, 7 }, { 7, 6, 5, 4 },
    { 4, 5, 1, 0 }, { 5, 6, 2, 1 }, { 7, 4, 0, 3 }
  };
  GLfloat v[8][3], tmp;
  GLint i;
  
  if (x0 > x1) {
    tmp = x0; x0 = x1; x1 = tmp;
  }
  if (y0 > y1) {
    tmp = y0; y0 = y1; y1 = tmp; 
  }
  if (z0 > z1) {
    tmp = z0; z0 = z1; z1 = tmp; 
  }
  v[0][0] = v[1][0] = v[2][0] = v[3][0] = x0;
  v[4][0] = v[5][0] = v[6][0] = v[7][0] = x1;
  v[0][1] = v[1][1] = v[4][1] = v[5][1] = y0;
  v[2][1] = v[3][1] = v[6][1] = v[7][1] = y1;
  v[0][2] = v[3][2] = v[4][2] = v[7][2] = z0;
  v[1][2] = v[2][2] = v[5][2] = v[6][2] = z1;
  
  for (i = 0; i < 6; i++) {
    glBegin(type);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][0]][0]);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][1]][0]);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][2]][0]);
    glNormal3fv(&n[i][0]);
    glVertex3fv(&v[faces[i][3]][0]);
    glEnd();
  }
}

/*---------------------------------------------------------*/
