/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

/* Search for connectivity */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "typedefs.h"
#include "find_things.h"

#define MIN_MALLOC 100

/*------------------------------------------------------------------------*/
void searchBonds(SYSTEM &system){
  if(system.bonds == NULL){
    system.bonds = (BONDS *)malloc(system.nconf*sizeof(BONDS));
  }
  int iconf;
  for(iconf=0;iconf<system.nconf;iconf++){
    system.bonds[iconf].search(system.natoms,iconf,system.atoms,system.coords);
  }
}

/*------------------------------------------------------------------------*/
void searchHbonds(SYSTEM &system){
  if(system.hbonds == NULL){
    system.hbonds = (HBONDS *)malloc(system.nconf*sizeof(HBONDS));
  }
  int iconf;
  for(iconf=0;iconf<system.nconf;iconf++){
    system.hbonds[iconf].search(iconf,system);
  }
}
/*------------------------------------------------------------------------*/
void search_def_att(ATOMS *atoms,int iatom)
{
  atoms[iatom].setdefatt();
}
