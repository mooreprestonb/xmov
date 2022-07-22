
#include <GL/gl.h>
#include "glsetvalues.h++"
#include "typedefs.h"
#include "bonds.h++"

const float BONDMAX=10.; // Maximum bond length to show
/*------------------------------------------------------------------------*/
BONDS::BONDS(void){ // constuctor 
  nbonds=-1;
  bond=(BOND *)NULL;
}
/*------------------------------------------------------------------------*/

void BONDS::set1(int n, BOND *bd){
  nbonds = n;
  bond = bd;
}

/*------------------------------------------------------------------------*/
void BOND::display(float x[],float xoff[DIM],ATOMS *atoms, SETVALUES &sv)
{
  GLfloat dx[DIM+1];

  GLUquadricObj *quadObj;
  quadObj = gluNewQuadric();
  
  if(sv.solid==1) gluQuadricDrawStyle(quadObj,(GLenum)GLU_FILL);
  else gluQuadricDrawStyle(quadObj,(GLenum)GLU_LINE);
  gluQuadricNormals (quadObj,(GLenum)GLU_SMOOTH);
  glPushMatrix();
  dx[0] = (atoms[_ib[1]].rcolor() + atoms[_ib[0]].rcolor())/2.;
  dx[1] = (atoms[_ib[1]].gcolor() + atoms[_ib[0]].gcolor())/2.;
  dx[2] = (atoms[_ib[1]].bcolor() + atoms[_ib[0]].bcolor())/2.;
  dx[3] = (atoms[_ib[1]].acolor() + atoms[_ib[0]].acolor())/2.;
  if(dx[3] >= 1.) {
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
  } else {
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
  }
  glColor4fv(dx);
  if ( !sv.bond_cyl ){
    int k;
    glPushMatrix();
    glBegin(GL_LINES);
    /*    dx[3] = ceil((atoms[_ib[1]].alpha() + atoms[_ib[0]].alpha())/2.); */
    for(k=0;k<DIM;k++) dx[k] = x[_ib[0]*DIM+k]-xoff[k];
    glVertex3fv(dx);
    for(k=0;k<DIM;k++) dx[k] = x[_ib[1]*DIM+k]-xoff[k];
    glVertex3fv(dx);
    glEnd();
    glPopMatrix();
  } else {
    int k;
    float r=0.;
    for(k=0;k<DIM;k++) dx[k] = x[_ib[1]*DIM+k]-x[_ib[0]*DIM+k];
    for(k=0;k<DIM;k++) r += dx[k]*dx[k];
    r = sqrt(r);
    if(r==0){
      fprintf(stderr,"Warning: zero bond length between %d and %d\n",
	      _ib[0],_ib[1]);
    } else if (r>BONDMAX) {
      fprintf(stderr,"Warning: bond length > BONDMAX(%g) between %d and %d\n",
	      BONDMAX,_ib[0],_ib[1]);
    } else {
      GLfloat dt[DIM];
      float a = acos(dx[1]/r)*180./M_PI;        
      for(k=0;k<DIM;k++) dt[k] = x[_ib[0]*DIM+k]-xoff[k];
      glTranslatef(dt[0],dt[1],dt[2]);
      glRotatef(-a+180,-dx[2]/r,0.0,dx[0]/r);
      glPushMatrix();
      glRotatef(90.0, 1.0, 0.0, 0.0);
      gluCylinder(quadObj, sv.bond_radius, sv.bond_radius, r, 
		  sv.cylinder_quality, 2);
      glPopMatrix();
    }
  }
  glPopMatrix ();
  gluDeleteQuadric(quadObj);
}

/*--------------------------------------------------------------------*/
void displayBonds(int nb,BOND *bond,float *x,ATOMS *atoms,SETVALUES &sv)
{
  int i;
  float xoff[3];

  glLineWidth((GLfloat)1.);

  for(i=0;i<DIM;i++) xoff[i] = sv.xoff[i];

  if(sv.solid) glShadeModel(GL_SMOOTH);
  else glShadeModel(GL_FLAT);

  for(i=0;i<nb;i++){
    
//     junk = bond[i].ishow();
//     fprintf(stdout,"%d:%d|",junk,i);

    if(bond[i].ishow() == 1)
      {
	int ib = bond[i].ib();
	int jb = bond[i].jb();
	if(atoms[ib].ishow() && atoms[jb].ishow()){
	  bond[i].display(x,xoff,atoms,sv);
	}/*end if ishow*/
      }/*if bonds*/
  }/*end for*/
}/*end fxn*/

/*------------------------------------------------------------------------*/
int BONDS::search(int natoms,int iconf,ATOMS *atoms,COORDS *coords)
{
  int i,j;
  int ibonds,ibmax;
  float dx[3],r,rmin;

  printf("Finding Bonds with radial cutoff (config=%d):",iconf);
  fflush(stdout);

  /* count bonds */
  ibonds=0;
  ibmax = MIN_MALLOC;
  float *x = coords[iconf].getx();
  if(x == NULL){
    fprintf(stderr,"ERROR: in bonding routine x data points to NULL!\n");
    exit(1);
  }
  bond = (BOND *)malloc(ibmax*sizeof(BOND));

  for(i=0;i<natoms;i++){
    for(j=i+1;j<natoms;j++){
      rmin = atoms[i].vdw() + atoms[j].vdw();
      dx[0] = x[i*DIM  ] - x[j*DIM  ]; 
      dx[1] = x[i*DIM+1] - x[j*DIM+1]; 
      dx[2] = x[i*DIM+2] - x[j*DIM+2]; 
      r = sqrt(dx[0]*dx[0]+dx[1]*dx[1]+dx[2]*dx[2]);
      if(r<rmin) {
	bond[ibonds].set(i,j);
	ibonds++;
	if(ibonds==ibmax){
	  ibmax+=MIN_MALLOC;
	  bond = (BOND *)realloc(bond,ibmax*sizeof(BOND));
	}
      }
    }
  }
  printf("%d\n",ibonds);
  nbonds = ibonds;
  bond = (BOND *)realloc(bond,ibonds*sizeof(BOND));
#ifdef DEBUG
  for(i=0;i<nbonds;i++){printf("CONECT %d %d\n",bond[i].ib(),bond[i].jb());}
#endif
  fflush(stdout);
  return nbonds;
}
