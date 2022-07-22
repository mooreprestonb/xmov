/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include "glsetvalues.h++"
#include "typedefs.h"
#include <GL/gl.h>

#ifndef LIGHTING_H_INCLUDED

extern void lighting(SETVALUES &);
extern void light_init(void);
extern void set_bcolor(GLfloat [4]);

#define MAX_LIGHT 10

#define LIGHTING_H_INCLUDED
#endif
