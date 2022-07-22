
#ifndef HBOND_H
#define HBOND_H

//#include <cmath>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "typedefs.h"
#include "coords.h++"
#include "atoms.h++"
#include "glsetvalues.h++"

#ifndef DIM
#define DIM 3
#endif

const int MIN_MALLOC=100;

const double RXHBMAX=1.5;
const double RXHMAX=2.5;
const double RXXMAX=3.6;
const double ARCCOS=.65;

// #define RXHBMAX 1.5
// #define RXHMAX 2.5
// #define RXXMAX 3.6
// #define ARCCOS .64 /* greater then 130 degrees */

class HBOND
{
public:
  int ih,jh,kh;
  // HBOND(void){} default construtor
  // ~HBOND(void){} default destructor
  void set(int i,int j,int k){ih=i;jh=j;kh=k;};
  void display(float [],float [3],ATOMS *);
};

class HBONDS
{
private:
  float rxhbmax,rxhmax,rxxmax,arccos;
public:
  int nhb;
  HBOND *hb;
  HBONDS(void); //constructor
  // ~HBONDS(void); destructor
  int search(int,struct SYSTEM &);
  void setvalues(float,float,float,float);
  void setvalues(void);
};

void displayHydro(int iconf,SYSTEM &,SETVALUES &);

#endif
