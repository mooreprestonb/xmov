
#include <GL/gl.h>
#include "glsetvalues.h++"
#include "typedefs.h"
#include "hbonds.h++"

// #define DEBUG
/*------------------------------------------------------------------------*/
void HBONDS::setvalues(float rxhb,float rxh ,float rxx,float arccosn){
  rxhbmax = rxhb;  rxhmax = rxh;  rxxmax = rxx;  arccos = arccosn;
}

void HBONDS::setvalues(void){
  setvalues(RXHBMAX,RXHMAX,RXXMAX,ARCCOS);
}

HBONDS::HBONDS(void){ // constuctor 
  nhb=-1;
  hb=(HBOND *)NULL;
  setvalues();
}

int HBONDS::search(int iconf,struct SYSTEM & system)
{
  int i,j,k,l,n;
  int ibmax,it;
  float dx1[3],dx2[3],r,r1,r2;

  int natoms = system.natoms;
  ATOMS *atoms = system.atoms;
  int nbonds = system.bonds[iconf].nbonds;
  float *x = system.coords[iconf].getx();
  BOND *bond = system.bonds[iconf].bond;

  fprintf(stdout,"Finding Hydrogen Bonds (config = %d):",iconf);
  fflush(stdout);
  nhb = 0;
  ibmax = MIN_MALLOC;

  hb = (HBOND *)malloc(ibmax*sizeof(HBOND));
  
  /* count hbonds */
  for(i=0;i<natoms;i++){
    it = atoms[i].itype();
    if(it==8 || it==9 || it==17 || it==7 || it==16 || it==15){
      for(n=0;n<nbonds;n++){
	j = -1;
	if(bond[n].ib() == i && atoms[bond[n].jb()].itype()==1) j=bond[n].jb();
	if(bond[n].jb() == i && atoms[bond[n].ib()].itype()==1) j=bond[n].ib();
	if(j==-1) continue;
	for(k=0;k<natoms;k++){
	  if(k==0) continue;
	  it = atoms[k].itype();
	  if(it==8||it==9||it==17||it==7||it==16||it==15){
	    for(l=0;l<DIM;l++) dx2[l] = x[i*DIM+l]- x[k*DIM+l];
	    for(l=0,r2=0.;l<DIM;l++) r2+= dx2[l]*dx2[l];
	    r2 = sqrt(r2);
	    if(r2<RXXMAX){
#ifdef DEBUG
	      printf("Hbonds almost found between %d and %d - %g\n",i,k,r2);
#endif
	      for(l=0;l<DIM;l++) dx1[l] = x[i*DIM+l]-x[j*DIM+l];
	      for(l=0,r1=0.;l<DIM;l++) r1 += dx1[l]*dx1[l];
	      r1 = sqrt(r1);
	      for(l=0,r=0;l<DIM;l++) r += dx1[l]*dx2[l];
	      r /= (r2*r1);
	      if(r1<RXHBMAX && r > ARCCOS){
#ifdef DEBUG
		printf("yes %d %d %d %g %g %g\n",i,j,k,r1,r2,r);
#endif
		for(l=0;l<DIM;l++) dx2[l] = x[j*DIM+l]-x[k*DIM+l];
		for(l=0,r2=0.;l<DIM;l++) r2 += dx2[l]*dx2[l];
		r2 = sqrt(r2);
		if(r2<RXHMAX){
		  hb[nhb].set(i,j,k);
		  nhb++;
		  if(nhb == ibmax){
		    ibmax += MIN_MALLOC;
		    hb=(HBOND*)realloc(hb,ibmax*sizeof(HBOND));
		  } /* end if for realloc */
		} /* end if within vdw radii */
#ifdef DEBUG
		printf("but not %d %d %d %g %g %g\n",i,j,k,r1,r2,r);
#endif
	      } /* end if correct type */
	    } /* loop over third atoms */
          } /* if second atom is hydrogen */
        } /* loop over atoms */
      } /* if correct type */
    } /* loop over atoms */
  }
  printf("%d\n",nhb);
#ifdef DEBUG
  for(l=0;l<nhb;l++){
    printf("%d, %d %s - %d %s - %d %s\n",l,
	   hb[l].ih,atoms[hb[l].ih].atomName(),
	   hb[l].jh,atoms[hb[l].jh].atomName(),
	   hb[l].kh,atoms[hb[l].kh].atomName());
  }
#endif
  return nhb;
}

// #define RHBOND .6
// #define GHBOND 1.
// #define BHBOND .6
#define RHBOND .5
#define GHBOND .2
#define BHBOND .5

void HBOND::display(float x[],float off[3],ATOMS *atoms)
{
  int i,k[3];

  if(atoms[ih].ishow() || atoms[jh].ishow() || atoms[kh].ishow()){
    k[0] = ih*DIM;  k[1] = jh*DIM; k[2] = kh*DIM;
    
    glColor3f(RHBOND, GHBOND, BHBOND);
    
    glBegin(GL_LINE_STRIP);
    for(i=0;i<3;i++)
      glVertex3f(x[k[i]]-off[0],x[k[i]+1]-off[1],x[k[i]+2]-off[2]);   
    glEnd();
  }
}
/*--------------------------------------------------------------------*/
void displayHydro(int iconf,SYSTEM &system,SETVALUES &sv)
{
  int i;
  GLfloat xoff[3];
  GLfloat *x;
  
  if(system.hbonds==NULL) {
    system.hbonds = new HBONDS[system.nconf];
  }
  if (system.hbonds[iconf].nhb==-1) system.hbonds[iconf].search(iconf,system);
  glLineWidth(2.0);

  for(i=0;i<DIM;i++) xoff[i] = sv.xoff[i];

  x = (system.coords[iconf]).getx();
  
  HBOND *hb = system.hbonds[iconf].hb;
  glPushMatrix();
  for(i=0;i<system.hbonds[iconf].nhb;i++) hb[i].display(x,xoff,system.atoms);
  glPopMatrix();
}
