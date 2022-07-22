/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef STEREO_H_INCLUDED

#include "typedefs.h"

extern void redraw(void);
void stereoDrawBuffer(GLenum mode);
void stereoClear(GLbitfield mask);
void stereoMakeCurrent(Display *dpy, Window win, GLXContext ctx);
void stereoEnable(void);
void stereoDisable(void);
void stereoInit(GLboolean usingStereoVisual, char *stereoCmd, char *restoreCmd);
void stereoDone(void);

void stereoFrustum(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
		   GLfloat near, GLfloat far, GLfloat eyeDist, 
		   GLfloat eyeOffset);
void stereoOrtho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
		 GLfloat near, GLfloat far, GLfloat eyeDist, 
		 GLfloat eyeOffset);

#define STEREO_H_INCLUDED
#endif
