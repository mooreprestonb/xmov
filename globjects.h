/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef GLOBJECTS_H_INCLUDED

#include <GL/gl.h>

void Sphere (GLdouble radius,int quality);
void Cylinder (GLdouble radius, GLdouble height, int quality);
void Cone(GLdouble radius, GLdouble height, int quality);
void WireBox (GLdouble width, GLdouble height, GLdouble depth);
void SolidBox (GLdouble width, GLdouble height, GLdouble depth);

#define GLOBJECTS_H_INCLUDED
#endif
