// class definitions for stereo and setvalues

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#ifndef SETVALUES_H
#define SETVALUES_H

class STEREO 
{
 public:
  int enabled,useSGIStereo;
  Display *currentDisplay;
  Window currentWindow;
  GLXContext currentContext;
  GLenum currentDrawBuffer;
  int currentStereoBuffer;
  char *stereoCommand;
  char *restoreCommand;
};

class SETVALUES 
{
 public:
  char *setfile,*attfile,*confile;
  int frame,disp_list;
  int light_spin,light_intensity;
  int stereoEnable,iperspective;
  int show_movie,icell,iatom,iperd,iperda;
  int rot_type,antialias_off,movie_type;
  int bonds,bond_cyl,hydro;
  int vector_cyl,vectors,solid,polygons, ribbons;
  int brot,xbegin,ybegin;
  int skip;
  int berase;
  XColor c_col; //Cell line color
  XColor top_left,bottom_left,bottom_right,top_right; //Colors for Gradient Background
  GLint sphere_quality,cylinder_quality;
  GLfloat scalefactor,bond_radius,transx,transy,transz,gamma;
  GLfloat xoff[3],sphere_scale;
  GLfloat rot_mat[16],cquat[4],aquat[4];
  GLfloat background[4];
  GLfloat line_range[2],line_granularity,line_width;
  GLfloat cell_line_width;
  STEREO stereo;
};


#endif
