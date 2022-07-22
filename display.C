/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

/* #define CELL0 to have the corner of the cell at (0,0,0) */
/* #define CYLINDER_VEC big cylinder for alpha helix viewing */
/* #define DEBUG */

#include "display.h"
#include "lighting.h"
#include "find_things.h"
#include "util_math.h"

extern void mal_verify(int);
extern void update_data_win(SETVALUES &);
extern void periodic(SYSTEM &,SETVALUES &);

float *sphere_tris(int maxlevel);

/***********************************************************************/
void reshape(SETVALUES &sv,int w,int h)
{
  GLfloat fovy,fov2,aspect,near,far,width,height,left,right,top,bottom;
  width=(GLfloat) w;
  height=(GLfloat) h;
  fovy = 40.;
  aspect = width/height;
  near = 1.;
  far = 10.;

  fov2 = ((fovy*M_PI) / 180.0) / 2.0;
  top = near / (cos(fov2) / sin(fov2));
  bottom = -top;  
  right = top * aspect;
  left = -right;

  glViewport(0, 0, w, h);
  
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  
  if(sv.iperspective){
    /* gluPerspective(40.0, (GLfloat) w/(GLfloat) h, 1.0, 10.0);*/
    glFrustum(left, right, bottom, top, near, far);
    printf("perspective\n");
  } else {
    aspect=1./aspect;
#ifdef ORTHOS
    if(w<=h) glOrtho(-1.0,1.0,-1.0*aspect,1.0*aspect,-10,10);
    else     glOrtho(-1.0*aspect,1.0*aspect,-1.0,1.0,-10,10);
#else
    glOrtho(left, right, bottom, top, near, far);
#endif
  }
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  glTranslatef (0.0, 0.0, -3.0);  
}


static int compare(const void *a, const void *b)
{
  DepthIndex *p1 = (DepthIndex *) a;
  DepthIndex *p2 = (DepthIndex *) b;
  float diff = p2->depth - p1->depth;

  if (diff > 0.0) {
    return 1;
  } else if (diff < 0.0) {
    return -1;
  } else {
    return 0;
  }
}

/**********************************************************************/
void gen_display_list(SYSTEM &system,DLIST &dlist,SETVALUES &sv)
{
  static int istart=1;
  int iframe;

  iframe = sv.frame;
  
  if ( istart ) {
    if ( (dlist.nlist  = glGenLists(system.nconf))==0 ) {
      fprintf(stderr,"ERROR: can't generate atoms lists!\n");
      exit(1);
    }
    if ( (dlist.nlist_cell  = glGenLists(system.nconf))==0 ) {
      fprintf(stderr,"ERROR: can't generate cell lists!\n");
      exit(1);
    }
    if ( (dlist.nlist_bonds = glGenLists(system.nconf))==0 ) {
      fprintf(stderr,"ERROR: can't generate bond lists!\n");
      exit(1);
    }
    istart=0;
  }
   
  printf("Creating display list for frame %d .",iframe); fflush(stdout);
  printf(".");  fflush(stdout);
  if ( sv.bonds ) {
    glNewList(dlist.nlist_bonds+iframe,GL_COMPILE);
    int nb = system.bonds[iframe].nbonds;
    BOND *bond = (system.bonds[iframe]).bond;
    float *x = (system.coords[iframe]).getx();
    displayBonds(nb,bond,x,system.atoms,sv);
    glEndList();
  }/*end if*/
  
  glNewList(dlist.nlist+iframe,GL_COMPILE);
  displayAtoms(iframe,system.natoms,system.atoms,system.coords,sv);
  glEndList();

  printf(".");  fflush(stdout);
  glNewList(dlist.nlist_cell+iframe,GL_COMPILE);
  display_cell(system.cell[iframe],sv,system.jscell);
  glEndList();
    
  printf(".");  fflush(stdout);
  printf(" Done\n");
  system.idisp[iframe] = 1;

}

/******************************************************************/

#define ZOFF_EXTRA 10000
int sortTriangles(int nconf,SYSTEM *system,SETVALUES &sv)
{
  int i,npol;
  GLfloat xoff[DIM],*x1,*y1,*z1,*x2,*y2,*z2,*x3,*y3,*z3;
  GLfloat *feed,avgdepth,zoff_extra;

  zoff_extra = ZOFF_EXTRA;
  for(i=0;i<DIM;i++) xoff[i] = sv.xoff[i];
  x1 = system->polygons.x1[nconf];  y1 = system->polygons.y1[nconf];  z1 = system->polygons.z1[nconf];
  x2 = system->polygons.x2[nconf];  y2 = system->polygons.y2[nconf];  z2 = system->polygons.z2[nconf];
  x3 = system->polygons.x3[nconf];  y3 = system->polygons.y3[nconf];  z3 = system->polygons.z3[nconf];

  feed=(GLfloat *)calloc(system->npolygons*11,sizeof(GLfloat));
  
  glFeedbackBuffer(system->npolygons*sizeof(GLfloat)*11,GL_3D,feed);
  glRenderMode(GL_FEEDBACK);
  glPushMatrix();
  glScaled(.3,.3,.3);

  glDepthMask(GL_FALSE);
  glTranslatef(0,0,-zoff_extra);
  
  glBegin(GL_TRIANGLES);
  for(i=0;i<system->npolygons;i++){
    glVertex3f(x1[i]-xoff[0],y1[i]-xoff[1],z1[i]-xoff[2]);  
    glVertex3f(x2[i]-xoff[0],y2[i]-xoff[1],z2[i]-xoff[2]);
    glVertex3f(x3[i]-xoff[0],y3[i]-xoff[1],z3[i]-xoff[2]);  
  }
  glEnd();
  glTranslatef(0,0,zoff_extra);
  glPopMatrix();
  npol=glRenderMode(GL_RENDER);
  npol/=11;
  printf("npol=%d\n",npol);
  if (npol != system->npolygons) printf("increase zoff_extra\n");
  /* for(i=0;i<5;i++) printf("%d %f \n",i,z1[i]); */
  

  for(i=0;i<system->npolygons;i++){
    system->polygons.indx[i].index=i;
    avgdepth=feed[11*i+4]+feed[11*i+7]+feed[11*i+10];
    system->polygons.indx[i].depth=avgdepth;
  }
/*  for(i=0;i<5;i++) {
    printf("%d %f \n",system->polygons.indx[i].index,system->polygons.indx[i].depth);
    }*/
  qsort(system->polygons.indx,system->npolygons , sizeof(DepthIndex), compare);
  printf("\n");
  
  /* for(i=system->npolygons-1;i>system->npolygons-5;i--) {
    printf("%d %f \n",system->polygons.indx[i].index,system->polygons.indx[i].depth);
    }*/
  glDepthMask(GL_TRUE);

  free(feed);

  return i;
  
}


/***********************************************************************/
void display_polygons(int nconf,SYSTEM *system,SETVALUES &sv)
{
  int i,j,npol;
  GLfloat xoff[DIM],xn,yn,zn,an;
  GLfloat *x1,*y1,*z1,*x2,*y2,*z2,*x3,*y3,*z3;
  GLfloat *nx1,*ny1,*nz1,*nx2,*ny2,*nz2,*nx3,*ny3,*nz3;

  if(nconf >= system->polygons.nconf) nconf = system->polygons.nconf-1;
  for(i=0;i<DIM;i++) xoff[i] = sv.xoff[i];
  x1 = system->polygons.x1[nconf];
  y1 = system->polygons.y1[nconf];
  z1 = system->polygons.z1[nconf];
  x2 = system->polygons.x2[nconf];
  y2 = system->polygons.y2[nconf];
  z2 = system->polygons.z2[nconf];
  x3 = system->polygons.x3[nconf];
  y3 = system->polygons.y3[nconf];
  z3 = system->polygons.z3[nconf];
  nx1 = system->polygons.nx1[nconf];
  ny1 = system->polygons.ny1[nconf];
  nz1 = system->polygons.nz1[nconf];
  nx2 = system->polygons.nx2[nconf];
  ny2 = system->polygons.ny2[nconf];
  nz2 = system->polygons.nz2[nconf];
  nx3 = system->polygons.nx3[nconf];
  ny3 = system->polygons.ny3[nconf];
  nz3 = system->polygons.nz3[nconf];
  glLineWidth(2.0);
  
  npol = sortTriangles(nconf,system,sv);
  glPushMatrix();
  
  /*  glDisable(GL_LIGHTING);*/
  glEnable(GL_BLEND);
  // glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
  glShadeModel(GL_SMOOTH);
  
  for(i=0;i<npol;i++){
    if(sv.solid){
      glBegin(GL_TRIANGLES);
    } else {
      // glBegin(GL_LINE_STRIP);
      glBegin(GL_LINE_LOOP);
    }

    j=system->polygons.indx[i].index;

    xn = (system->polygons.rcolor[j]);
    yn = (system->polygons.gcolor[j]);
    zn = (system->polygons.bcolor[j]);
    an = (system->polygons.acolor[j]);
    glColor4f(xn, yn, zn,an);
    
    // printf("alpha = %g\n",an);
    glNormal3f(nx1[j],ny1[j],nz1[j]);
    glVertex3f(x1[j]-xoff[0],y1[j]-xoff[1],z1[j]-xoff[2]);  
    glNormal3f(nx2[j],ny2[j],nz2[j]);
    glVertex3f(x2[j]-xoff[0],y2[j]-xoff[1],z2[j]-xoff[2]);
    glNormal3f(nx3[j],ny3[j],nz3[j]);
    glVertex3f(x3[j]-xoff[0],y3[j]-xoff[1],z3[j]-xoff[2]);  
    glEnd();
  }
  // glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
  /*  glEnable(GL_LIGHTING);*/ 
  glPopMatrix();
}


/***********************************************************/
void display_ribbons(int nconf,SYSTEM *system,SETVALUES &sv)
{
  int i;
  GLfloat xoff[DIM],xn,yn,zn;
  GLfloat *x1,*y1,*z1;
  
  if(nconf > system->ribbons.nconf) nconf = system->ribbons.nconf-1;

  for(i=0;i<DIM;i++) xoff[i] = sv.xoff[i];

  x1 = system->ribbons.x1[nconf];
  y1 = system->ribbons.y1[nconf];
  z1 = system->ribbons.z1[nconf];
  glLineWidth(2.0);
  
  glPushMatrix();
  glBegin(GL_LINE_STRIP);
  for(i=0;i<system->nribbons;i++){
    xn = (system->ribbons.rcolor[i]);
    yn = (system->ribbons.gcolor[i]);
    zn = (system->ribbons.bcolor[i]);
    glColor3f(xn, yn, zn);
    
    glVertex3f(x1[i]-xoff[0],y1[i]-xoff[1],z1[i]-xoff[2]);  
  }
  glEnd();
  glPopMatrix();
}
/*-------------------------------------------------------*/
void drawjline(float xoff[3],float vx1[3],float vx2[3])
{
  glVertex3f(vx1[0]-xoff[0],vx1[1]-xoff[1],vx1[2]-xoff[2]);   
  glVertex3f(vx2[0]-xoff[0],vx2[1]-xoff[1],vx2[2]-xoff[2]);   
}

void display_cell(CELLS cell,SETVALUES &sv,JSCELLS *jscell)
{
  int i,l,k;
  GLfloat xoff[DIM],vx1[DIM],vx2[DIM];
  
  glDisable(GL_LIGHTING);
  glColor3f(0.0,0.6,0.8);  /* cyan */
  glLineWidth(1.0);
  
  glPushMatrix();
  glBegin(GL_LINES);

  switch(sv.movie_type){
  case 1:
    for(i=0;i<DIM;i++) xoff[i] = sv.xoff[i]; 
    for(i=0;i<jscell->nlines;i++){
      GLfloat *jx = jscell->x;
      int j = (jscell->corn1[i]-1)*DIM;
      int k = (jscell->corn2[i]-1)*DIM;
      glVertex3f(jx[j]-xoff[0],jx[j+1]-xoff[1],jx[j+2]-xoff[2]); 
      glVertex3f(jx[k]-xoff[0],jx[k+1]-xoff[1],jx[k+2]-xoff[2]); 
    }
    break;
  case 2:
  case 0:    
#ifdef CELL0
    for(i=0;i<DIM;i++) xoff[i] = sv.xoff[i]; 
#else /* subtract off sv.xoff from each to move cell with offset */
    for(i=0;i<DIM;i++) xoff[i] = (cell[i]+cell[i+3]+cell[i+6])/2.;
#endif

#ifdef DEBUG
    printf("CELL offsets = %g %g %g\n",xoff[0],xoff[1],xoff[2]);
#endif

    /* line 1,2,3 */
    for(l=0;l<DIM;l++) vx1[l] = 0;
    for(k=0;k<DIM;k++){
      for(l=0;l<DIM;l++) vx2[l] = cell[l*3+k];
      drawjline(xoff,vx1,vx2);
    }

    /* line 4,5,6 */
    for(l=0;l<DIM;l++) vx1[l] = cell[l*3] + cell[l*3+1] + cell[l*3+2];
    vx2[0]=cell[0]+cell[1]; vx2[1]=cell[3]+cell[4]; vx2[2]=cell[6]+cell[7];
    drawjline(xoff,vx1,vx2);
    vx2[0]=cell[1]+cell[2]; vx2[1]=cell[4]+cell[5]; vx2[2]=cell[7]+cell[8];
    drawjline(xoff,vx1,vx2);
    vx2[0]=cell[0]+cell[2]; vx2[1]=cell[3]+cell[5]; vx2[2]=cell[6]+cell[8];
    drawjline(xoff,vx1,vx2);
      
    /* line 7,8 */
    vx1[0] = cell[0]; vx1[1] = cell[3];  vx1[2] = cell[6];
    vx2[0] = cell[0]+cell[1]; vx2[1]=cell[3]+cell[4];  vx2[2]=cell[6]+cell[7];
    drawjline(xoff,vx1,vx2);
    vx2[0] = cell[0]+cell[2]; vx2[1]=cell[3]+cell[5];  vx2[2]=cell[6]+cell[8];
    drawjline(xoff,vx1,vx2);

    /* line 9,10 */
    vx1[0] = cell[1]; vx1[1] = cell[4];  vx1[2] = cell[7];
    vx2[0] = cell[0]+cell[1]; vx2[1]=cell[3]+cell[4];  vx2[2]=cell[6]+cell[7];
    drawjline(xoff,vx1,vx2);
    vx2[0] = cell[1]+cell[2]; vx2[1]=cell[4]+cell[5];  vx2[2]=cell[7]+cell[8];
    drawjline(xoff,vx1,vx2); 
    
    /* line 11,12 */
    vx1[0] = cell[2]; vx1[1] = cell[5]; vx1[2] = cell[8];
    vx2[0] = cell[0]+cell[2]; vx2[1]=cell[3]+cell[5];  vx2[2]=cell[6]+cell[8];
    drawjline(xoff,vx1,vx2); 
    vx2[0] = cell[1]+cell[2]; vx2[1]=cell[4]+cell[5];  vx2[2]=cell[7]+cell[8];
    drawjline(xoff,vx1,vx2); 
  } /* end switch */
  glEnd();
  glPopMatrix();
  glEnable(GL_LIGHTING);
}
/*-----------------------------------------------------------*/
void display(SETVALUES &sv,SYSTEM &system,DLIST &dlist)
{
  int f;
  
  f = sv.frame % system.nconf;
  sv.frame = f;
  
  if(sv.iperd) periodic(system,sv);

  if(sv.disp_list){
    /* printf("displaying frame %d (%d)\n",f,system.idisp[f]);mal_verify(1);*/
    if(system.idisp[f]==0) gen_display_list(system,dlist,sv);
  }
  
  glPushMatrix();
  if (sv.hydro) displayHydro(f,system,sv);
  if (sv.vectors){
    int iv = (f>=system.nvectors)?system.nvectors-1:f;
    displayVectors(system.vectors[iv],sv);
  }
  if(sv.disp_list){
    if (sv.icell) glCallList(dlist.nlist_cell+f);
    if (sv.bonds) glCallList(dlist.nlist_bonds+f);
    if (sv.iatom) glCallList(dlist.nlist+f);
  } else {
    if (sv.icell) display_cell(system.cell[f],sv,system.jscell);
    if (sv.bonds) {
      int nb = system.bonds[f].nbonds;
      BOND *bond = (system.bonds[f]).bond;
      float *x = (system.coords[f]).getx();
      displayBonds(nb,bond,x,system.atoms,sv);
    }
    if (sv.iatom) displayAtoms(f,system.natoms,system.atoms,system.coords,sv);
  }
  if (sv.polygons) display_polygons(f,&system,sv);
  if (sv.ribbons) display_ribbons(f,&system,sv);  
  glPopMatrix(); 
  
  /*
    printf("displayed frame %d (%d)\n",f,system->idisp[f]);  mal_verify(1);
  */
  update_data_win(sv);
}

