
#include "coords.h++"
#define DIM 3

/*!
  constructor
*/
COORDS :: COORDS(void)
{
  _natoms=0;
  _x = (float *)NULL;
  _f = (float *)NULL;
  for(int i=0;i<9;i++) _xbox[i] = 0;
}

/*!
  destructor
*/
COORDS :: ~COORDS(void)
{
  if(_x != (float *)NULL){delete[] _x;}
  if(_f != (float *)NULL){delete[] _f;}
}

/*!
  allocate the space for natoms
*/
float* COORDS :: setncoords(int natoms)
{
  _natoms = natoms;
  // if(x != (float *)NULL) free(x);
  _x = (float *)malloc(_natoms*DIM*sizeof(float));
  for(int i=0;i<_natoms*DIM;i++) _x[i] = 0.;
  return _x;
}

/*! 
  allocate the transparency factor for CG
*/
float* COORDS :: setnfact(int natoms)
{
  _f = (float *)malloc(_natoms*sizeof(float));
  for(int i=0;i<_natoms;i++) _f[i] = 0.;
  return _f;
}

/*! 
  set the box lengths all to the same lenght
*/
void COORDS :: setbox(float box){
  for(int i=0;i<9;i++) _xbox[i] = 0.0;
  _xbox[0] = _xbox[4] = _xbox[8] = box;
}

/*! 
  set the box lengths h1, h2 and h3
*/
void COORDS :: setbox(float h1,float h2, float h3)
{
  _xbox[0] = h1;  _xbox[4] = h2;  _xbox[8] = h3;
}

/*! 
  set the h matrix
*/
void COORDS :: setbox(float h1[9])
{
  int i;
  for(i=0;i<9;i++) _xbox[0] = h1[i];
}
  
