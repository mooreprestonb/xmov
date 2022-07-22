/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/


#include "glsetvalues.h++"
#include "typedefs.h"

#ifndef READ_THINGS_H_INCLUDED

void readInput(SYSTEM &,SETVALUES &);
void read_setfile(char *setfile,int *nspec,SPECIES *spec_root);
void read_topfile(SYSTEM &,int nspec,SPECIES *spec_root);
void read_attributes(char *,int,ATOMS *);
void read_jsmovie(char *filename,SYSTEM &system);

void setValues(SETVALUES &setvalues,SYSTEM &system);
void search_def_att(ATOMS *atoms,int iatom);
void free_att(int natt,ATTRIBUTES *att);
void readVectors(char *,int *,VECTS **);
void readVectorsAtoms(char *,int *,VECTS **,int, float **);
void read_polygons(char *,int *,POLYGONS *);
void read_n_polygons(char *,int *,POLYGONS *);
void read_ribbons(char *, int *, RIBBONS *);
void read_ellipse(char *,int ,float **);
void searchBonds(SYSTEM &system);

#define READ_THINGS_H_INCLUDED
#endif
