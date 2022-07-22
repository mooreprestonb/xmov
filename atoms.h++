/*! 
  \file atoms.h++
  \brief ATOMS class definition and functions   
*/

#include "glsetvalues.h++"
#include "coords.h++"

#ifndef _ATOMS_H
#define _ATOMS_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef DIM
const int DIM=3;
#endif

class ATOMS
{
private:
  bool _ishow; // is this atom displayed
  int _itype,_ispec,_imole,_iatom,_igroup; // atom indexes
  float _color[DIM+1]; // colors [rgba]
  float _radius,_vdw,*_ellipse; // shape and size
  float _amass,_qch,_beta; // mass, charge, trans att. 
  char *_atomName,*_groupName,*_moleName,*_specName; // names
public:
  ATOMS(void);//!< constructor
  ATOMS(const ATOMS &src){*this=src;} //!< constructor
  ~ATOMS(void); //!< destructor
  ATOMS & operator = (const ATOMS &src); //!< assignment operator
  void reset(); //! clear all variables
  void setellipse(float ell[16]); //!< set the ellipse matrix
  void display(float [],float [],float); //!< display the atoms
  bool ishow(void) {return _ishow;} //!< return ishow
  void ishow(const bool ishow) {_ishow = ishow;} //!< set ishow
  float radius(void) {return _radius;} //!< return radius
  void radius(float radius) {_radius = radius;} //!< set radius
  float beta(void) {return _beta;} //!< return beta
  void beta(float beta) {_beta = beta;} //!< set beta
  char *atomName(void) {return _atomName;} //!< return atom name
  void atomName(const char *src) { //!< set atom name (strip leading #)
    _atomName = strdup(src);
    /* fix if they start with a stupid number */
    if(_atomName[0] >= '0' && _atomName[0] <= '9') {
      for(int i=0;(unsigned)i<(strlen(_atomName)-1);i++){
	_atomName[i] = _atomName[i+1];
      }
    }
  }
  char *moleName(void) {return _moleName;} //!< return molecule name
  void moleName(const char *src) {_moleName = strdup(src);} //!< set mol name
  char *groupName(void) {return _groupName;} //!< return group name
  void groupName(const char *src) {_groupName = strdup(src);}//!<set group name
  char *specName(void) {return _specName;} //!< return species name
  void specName(const char *src) {_specName = strdup(src);}//!<set species name
  int ispec(void) {return _ispec;} //!< return species #
  void ispec(int ispec) {_ispec = ispec;} //!< set species #
  int imole(void) {return _imole;} //!< return molecule #
  void imole(int imole) {_imole = imole;} //!< set molecule #
  int iatom(void) {return _iatom;} //!< return atom #
  void iatom(int iatom) {_iatom = iatom;} //!< set atom #
  int igroup(void) {return _igroup;} //!< return group #
  void igroup(int igroup) {_igroup = igroup;} //!< set group #
  int itype(void) {return _itype;} //!< return type #
  void itype(int itype) {_itype = itype;} //!< set type 
  void color3(float fcolor[DIM]){ //!< set rgb colors
    for(int i=0;i<DIM;i++) _color[i] = fcolor[i];
  }
  void color4(float fcolor[DIM+1]){ //!< set rgb colors
    for(int i=0;i<DIM+1;i++) _color[i] = fcolor[i];
  }
  void color(float red,float green, float blue){ //!< set rgb colors
    _color[0] = red; _color[1] = green; _color[2] = blue;
  } 
  //!< set rgba colors
  void color(float red,float green, float blue, float alpha){ 
    _color[0] = red; _color[1] = green; _color[2] = blue; _color[3] = alpha;
  } 
  void  rcolor(float rcolor){_color[0] = rcolor;} //!< set red color
  float rcolor(void){return _color[0];} //!< return red color
  void  gcolor(float gcolor){_color[1] = gcolor;} //!< set green color
  float gcolor(void){return _color[1];} //!< return green color
  void  bcolor(float bcolor){_color[2] = bcolor;} //!< set blue color
  float bcolor(void){return _color[2];} //!< return blue color
  void acolor(float alpha) {_color[3] = alpha;} //!<set alpha ie transparency
  float acolor(void) {return _color[3];} //!< return alpha ie transparency
  void  vdw(float vdw){_vdw = vdw;} //!< set vdw radii
  float vdw(void){return _vdw;} //!< return vdw radii
  void  amass(float amass){_amass = amass;} //!< set mass
  float amass(void){return _amass;} //!< return mass
  void  qch(float qch){_qch = qch;} //!< set charge
  float qch(void){return _qch;} //!< return charge
  void printid(FILE *); //!< print id (name type etc) to file
  void print(FILE *); //!< print class to file
  void setdefatt(void); //!< set default attributes
};

// display atoms
void displayAtoms(int nconf,int natoms,ATOMS *,COORDS *,SETVALUES &);

#endif
