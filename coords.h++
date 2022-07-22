#ifndef _COORDS_H
#define _COORDS_H

#include <stdlib.h>

class COORDS
{
private:
  int _natoms;
  float _xbox[9];
  float *_x;
  float *_f;
public:
  COORDS(void);
  ~COORDS(void);
  int getatoms(void){return _natoms;};
  float* getxbox(void){return _xbox;};
  float* getx(void) {return _x;};
  float* getf(void) {return _f;};
  float* setncoords(int natoms);
  float* setnfact(int natoms);
  void setbox(float box);
  void setbox(float boxx,float boxy, float boxz);
  void setbox(float box[9]);
};

#endif
