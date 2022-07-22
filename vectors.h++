/*!
  \file vectors.h++
  file for the vector class
  (not a true vector but a display vector)

typedef struct VECTORS{
  int nconf;
  float **x1,**y1,**z1,**x2,**y2,**z2;
  float *rcolor,*gcolor,*bcolor;
  float scale;
} VECTORS;

*/

#include "atoms.h++"
#include "glsetvalues.h++"

#ifndef _VECTORS_H
#define _VECTORS_H

#ifndef DIM
const int DIM=3;
#endif

static const float defveccolors[3] = {0.5,1.0,1.0};

class VECT 
{
private:
  float _x[2*DIM];
  float _color[DIM];
public:
  VECT(void){
    int i;
    for(i=0;i<4*DIM;i++) _x[i] = 0.;
    for(i=0;i<DIM;i++) _color[i] = defveccolors[i];
  }
  ~VECT(void); // destructor
  void display(SETVALUES &);
  float * color(void){return _color;}
  void set(const float[2*DIM]);
  void print(FILE *);
};

class VECTS 
{
private:
  int _nvects;
  VECT *_vect;
public: // variables
  static float linewidth;
  static float scale;
public: // functions
  VECTS(void); //constructor
  ~VECTS(void); //destructor
  VECT *vect(void){return _vect;}
  int nvects(void){return _nvects;}
  void set(int);
};

// prototypes of functions that use VECTS
void displayVectors(VECTS &,SETVALUES &sv);
#endif
