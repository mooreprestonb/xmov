#ifndef BOND_H
#define BOND_H

//#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include "coords.h++"
#include "atoms.h++"
#include "glsetvalues.h++"

#define DIM 3
#define MIN_MALLOC 100

class BOND
{
private:
  int _ib[2], _ishow;
public:
  BOND(void){_ib[0]=-1; _ib[1]= -1; _ishow=1;} //default construtor
  // ~ATOMS(void){} default destructor
  int ib(void){return _ib[0];}
  int jb(void){return _ib[1];}
  int ishow(void){return _ishow;}
  void ishow(int i){_ishow=i;}
  void set(int i,int j)
  {
    _ib[0]=i;
    _ib[1]=j;
    _ishow=1;
  }
  void display(float [],float [DIM],ATOMS *,SETVALUES &);
};

class BONDS
{
public:
  int nbonds;
  BOND *bond;
  BONDS(void); //constructor
  // ~BONDS(void); destructor
  int search(int, int, ATOMS *, COORDS *);
  void set1(int, BOND *);
  void setvalues(void);
};

void displayBonds(int nb,BOND *bond,float *x,ATOMS *atoms,SETVALUES &sv);

#endif

