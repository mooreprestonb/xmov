/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include "glsetvalues.h++"
#include "typedefs.h"

#ifndef PICK_H_INCLUDED
#define PICK_H_INCLUDED

int pick(int x,int y,int w,int h,SETVALUES &);
int showAll(int ishow);
int showSpec(int ishow,int ispec);
int showSpecType(int ishow,int ispec,int itype);
int showSpecAtomName(int ishow,int ispec,char *name);
int showSpecMol(int ishow,int ispec, int imol);
int showSpecGroup(int ishow,int ispec,int igroup);
int showGroup(int ishow,int igroup);
int showGroupName(int ishow,char *name);
int showSpecGroupName(int ishow,int ispec,char *name);
int showAtom(int ishow,int ipick);
int showType(int ishow,int itype);
int showAtomName(int ishow,char *name);
int colorSpecMol(int, int, float [4]);
void pic_show(int);
void bond_show(int);
#endif




