/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef _TYPEDEFS_H_INCLUDED
#define _TYPEDEFS_H_INCLUDED

#define MAXLINE 512
#define DIM 3
typedef char WORD[MAXLINE];
typedef float CELLS[9];

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#include "coords.h++"
#include "atoms.h++"
#include "hbonds.h++"
#include "bonds.h++"
#include "vectors.h++"

typedef struct keys {
   WORD keyword,keyarg;
   struct keys *next; 
} KEY;

typedef struct DepthIndex {
  int index;
  float depth;
} DepthIndex;

typedef struct ATTRIBUTES {
  WORD atom;
  int anum;
  float red,green,blue,radius,vdw,alpha;  
  struct ATTRIBUTES *next;
} ATTRIBUTES;

typedef struct jscell {
   int *corn1,*corn2,nlines,ncorners;
   float *x;
} JSCELLS;

typedef struct species {
   WORD filename,thermopt,name;
   int nmol,molidx,napm,nbond,nbend,ntors,n14,nbondx,npath;
   struct species *next; 
} SPECIES;

typedef struct satom_topol {
  int atm_idx;
  WORD type,group;
  float mass,charge;
} ATOM_TOPOL;


typedef struct POLYGONS{
  int nconf;
  float **x1,**y1,**z1,**x2,**y2,**z2,**x3,**y3,**z3;
  float **nx1,**ny1,**nz1,**nx2,**ny2,**nz2,**nx3,**ny3,**nz3;
  float *rcolor,*gcolor,*bcolor,*acolor;
  DepthIndex *indx;
} POLYGONS;

typedef struct RIBBONS{
  int nconf;
  float **x1,**y1,**z1;
  float *rcolor,*gcolor,*bcolor;
} RIBBONS;

typedef struct BBONE{
  int *nconf;
  float **x1,**y1,**z1;
  float *rcolor,*gcolor,*bcolor;
} BBONE;


typedef struct DLIST {
  int nlist;
  int nlist_bonds;
  int nlist_cell;
  int nlist_vectors;
} DLIST;

typedef struct SYSTEM {
  int nconf,natoms,nvectors,npolygons,nribbons,iskip,*idisp;
  float dt;
  ATOMS *atoms;
  BONDS *bonds;
  VECTS *vectors;
  HBONDS *hbonds;
  POLYGONS polygons;
  RIBBONS ribbons;   
  CELLS *cell;
  JSCELLS *jscell;
  COORDS *coords;
} SYSTEM;

#define md_stdout(A) fprintf(stdout,"%s\n",A);
#define md_error(A)  {fprintf(stderr,"ERROR: %s\n",A);exit(1);}
#define md_warning(A) fprintf(stderr,"WARNING: %s\n",A);

#endif
