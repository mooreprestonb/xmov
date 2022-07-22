
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include <Xm/DrawingA.h>

#include "stereo.h"
#include "display.h"
#include "lighting.h"

extern Display *dpy;
extern Window win;
extern Widget da;

extern SYSTEM sys;
extern SETVALUES sv;
extern DLIST dl;

#ifdef SGI
#include <X11/extensions/SGIStereo.h>
#endif

void redraw(void)
{
  Dimension width_d,height_d;
  float fovy,aspect,near,far,width,height; 
  GLfloat fov2,left,right,bottom,top;
  GLfloat eyeDist = 2.0;
  GLfloat eyeOffset = 0.05;

  XtVaGetValues(da,XmNwidth,&width_d,XmNheight,&height_d,NULL);

  width = (float)width_d;
  height = (float)height_d;
  fovy = 40.;
  aspect = width/height;
  near = 1.;
  far = 10.;

  fov2 = ((fovy*M_PI) / 180.0) / 2.0;
  top = near / (cos(fov2) / sin(fov2));
  bottom = -top;  
  right = top * aspect;
  left = -right;
  
 
  lighting(sv);

  glPushMatrix();
  glTranslatef(sv.transx,sv.transy,sv.transz);
  glMultMatrixf(sv.rot_mat);
  glScalef(sv.scalefactor,sv.scalefactor,sv.scalefactor);

  if (sv.stereo.enabled) {
    if (sv.stereo.useSGIStereo) {
      glViewport(0, 0, (int)width,(int) height/2);
    }
    /*
    ** Draw right-eye view.
    */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if(!sv.iperspective){
      stereoOrtho(left,right,bottom,top,near,far,eyeDist,eyeOffset);
    } else {
      stereoFrustum(left,right,bottom,top,near,far,eyeDist,eyeOffset);
    }
    glMatrixMode(GL_MODELVIEW);
    
    stereoDrawBuffer(GL_BACK_RIGHT);
    stereoClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    display(sv,sys,dl);

  } else {
    eyeOffset = 0.0;
  }
  
  /*
  ** Draw left-eye view. or default view
  */
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
    
  if(!sv.iperspective){
    stereoOrtho(left,right,bottom,top,near,far,eyeDist,-eyeOffset);
  } else {
    stereoFrustum(left, right, bottom, top, near, far, eyeDist, -eyeOffset);
  }
  glMatrixMode(GL_MODELVIEW);
  stereoDrawBuffer(GL_BACK_LEFT);
  stereoClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  display(sv,sys,dl);

  glPopMatrix();
  glXSwapBuffers(dpy,win);
}

/* call instead of glDrawBuffer */
void stereoDrawBuffer(GLenum mode)
{
#ifdef SGI
  if (sv.stereo.useSGIStereo) {
    sv.stereo.currentDrawBuffer = mode;
    switch (mode) {
    case GL_FRONT:
    case GL_FRONT_AND_BACK:
      /*
      ** Simultaneous drawing to both left and right buffers isn't
      ** really possible if we don't have a stereo capable visual.
      ** For now just fall through and use the left buffer.
      */
    case GL_LEFT:
    case GL_FRONT_LEFT:
      sv.stereo.currentStereoBuffer = STEREO_BUFFER_LEFT;
      mode = GL_FRONT;
      break;
    case GL_BACK:
    case GL_BACK_LEFT:
      sv.stereo.currentStereoBuffer = STEREO_BUFFER_LEFT;
      mode = GL_BACK;
      break;
    case GL_RIGHT:
    case GL_FRONT_RIGHT:
      sv.stereo.currentStereoBuffer = STEREO_BUFFER_RIGHT;
      mode = GL_FRONT;
      break;
    case GL_BACK_RIGHT:
      sv.stereo.currentStereoBuffer = STEREO_BUFFER_RIGHT;
      mode = GL_BACK;
      break;
    default:
      break;
    }
    if (sv.stereo.currentDisplay && sv.stereo.currentWindow) {
      glXWaitGL(); /* sync with GL command stream before calling X */
      XSGISetStereoBuffer(sv.stereo.currentDisplay,
          sv.stereo.currentWindow,sv.stereo.currentStereoBuffer);
      glXWaitX(); /* sync with X command stream before calling GL */
    }
  }
#endif
  glDrawBuffer(mode);
}

/* call instead of glClear */
void stereoClear(GLbitfield mask)
{
  if (sv.stereo.useSGIStereo) {
    GLenum drawBuffer = sv.stereo.currentDrawBuffer;
    switch (drawBuffer) {
    case GL_FRONT:
      stereoDrawBuffer(GL_FRONT_RIGHT);
      glClear(mask);
      stereoDrawBuffer(drawBuffer);
      break;
    case GL_BACK:
      stereoDrawBuffer(GL_BACK_RIGHT);
      glClear(mask);
      stereoDrawBuffer(drawBuffer);
      break;
    case GL_FRONT_AND_BACK:
      stereoDrawBuffer(GL_RIGHT);
      glClear(mask);
      stereoDrawBuffer(drawBuffer);
      break;
    case GL_LEFT:
    case GL_FRONT_LEFT:
    case GL_BACK_LEFT:
    case GL_RIGHT:
    case GL_FRONT_RIGHT:
    case GL_BACK_RIGHT:
    default:
      break;
    }
  }
  glClear(mask);
}

/* call after glXMakeCurrent */
void stereoMakeCurrent(Display *dpy, Window win, GLXContext ctx)
{
#ifdef SGI
  if (sv.stereo.useSGIStereo) {
    if (dpy && (dpy != sv.stereo.currentDisplay)) {
      int event, error;
      /* Make sure new Display supports SGIStereo */
      if (XSGIStereoQueryExtension(dpy, &event, &error) == False) {
	dpy = NULL;
      }
    }
    if (dpy && win && (win != sv.stereo.currentWindow)) {
      /* Make sure new Window supports SGIStereo */
      if (XSGIQueryStereoMode(dpy, win) == X_STEREO_UNSUPPORTED) {
	win = None;
      }
    }
    if (ctx && (ctx != sv.stereo.currentContext)) {
      GLint drawBuffer;
      glGetIntegerv(GL_DRAW_BUFFER, &drawBuffer);
      stereoDrawBuffer((GLenum) drawBuffer);
    }
    sv.stereo.currentDisplay = dpy;
    sv.stereo.currentWindow = win;
    sv.stereo.currentContext = ctx;
  }
#endif
}

/* call to turn on stereo viewing */
void stereoEnable(void)
{
  printf("Going to stereo mode\n");
  if (!sv.stereo.enabled && sv.stereo.stereoCommand) {
    printf("Issuing System Command \"%s\"\n",sv.stereo.stereoCommand);
    system(sv.stereo.stereoCommand);
  }
  sv.stereo.enabled = True;
}

/* call to turn off stereo viewing */
void stereoDisable(void)
{
  printf("Going to mono mode\n");
  if (sv.stereo.enabled && sv.stereo.restoreCommand) {
    system(sv.stereo.restoreCommand);
  }
  sv.stereo.enabled = False;
}

/* call before using stereo */
void stereoInit(GLboolean usingStereoVisual, char *stereoCmd, char *restoreCmd)
{
#ifdef SGI
  sv.stereo.useSGIStereo = !usingStereoVisual;
  sv.stereo.currentDisplay = NULL;
  sv.stereo.currentWindow = None;
  sv.stereo.currentContext = NULL;
  sv.stereo.currentDrawBuffer = GL_NONE;
  sv.stereo.currentStereoBuffer = STEREO_BUFFER_NONE;
  sv.stereo.enabled = False;
  if (sv.stereo.stereoCommand) { free(sv.stereo.stereoCommand); }
  sv.stereo.stereoCommand = stereoCmd ? strdup(stereoCmd) : NULL;
  if (sv.stereo.restoreCommand) {  free(sv.stereo.restoreCommand); }
  sv.stereo.restoreCommand = restoreCmd ? strdup(restoreCmd) : NULL;
#endif
}

/* call when done using stereo */
void stereoDone(void)
{
  stereoDisable();
  stereoInit(True, NULL, NULL);
}

/************************************************************************/

void stereoFrustum(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
		   GLfloat near, GLfloat far, GLfloat eyeDist, 
		   GLfloat eyeOffset)
{
  GLfloat eyeShift = (eyeDist - near) * (eyeOffset / eyeDist);
  
  glFrustum(left+eyeShift, right+eyeShift, bottom, top, near, far);
  glTranslatef(-eyeShift, 0.0, 0.0);
}

void stereoOrtho(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top,
		 GLfloat near, GLfloat far, GLfloat eyeDist, 
		 GLfloat eyeOffset)
{
  GLfloat eyeShift = (eyeDist - near) * (eyeOffset / eyeDist);
  
  glOrtho(left+eyeShift, right+eyeShift, bottom, top, near, far);
  glTranslatef(-eyeShift, 0.0, 0.0);
}
/************************************************************************/
