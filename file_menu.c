/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include <Xm/MainW.h>
#include <Xm/ScrolledW.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/FileSB.h>
#include <X11/keysym.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "typedefs.h"
#include "menuutil.h"
#include "display.h"
#include "read_things.h"
#include "event_func.h"
#include "file_menu.h"
#include "find_things.h"
#include "stereo.h"

extern Widget da,toplevel;

extern XtAppContext app;
extern Display *dpy;
extern Window win;

extern SYSTEM sys;
extern SETVALUES sv;
extern DLIST dl;

void printHelp()
{
  printf("\n");
  printf("Key Sequence    -> Action \n");
  printf("--------------------------\n");
  printf("Escape Key    -> Quit\n");
  printf("Space Bar     -> Start or stop movie\n");
  printf("Arrow Keys    -> Rotate (about xy)\n");
  printf("Page Up/down  -> Rotate (about z)\n");
  printf(" 1 or 2       -> Spin light source\n");
  printf(" I/i          -> Increase/Decrease light intensity\n");
  printf(" f/F          -> View next/previous frame\n");
  printf(" p            -> Toggle perspective\n");
  printf(" c            -> Toggle viewing cell box\n");
  printf(" Q/q          -> Increase/Decrease sphere and cylinder quality\n");
  printf(" K/k          -> Increase/Decrease line width\n");
  printf(" w            -> Toggle wire/solid objects\n");
  printf(" l            -> Toggle cylinder lines\n");
  printf(" Z/z          -> Scale up/down\n");
  printf(" a            -> Toggle periodic boundry conditions for molecules\n");
  printf(" A            -> Toggle periodic boundry conditions for atoms\n");
  printf(" Home         -> Reset values\n");
  printf(" r            -> Toggle rotation about fixed or body coordinates\n");
  printf(" s            -> smooth with antialiasing (doesn't always work)\n");
  printf(" N/n          -> Turn on/off display of bonds for species/types/molecules/etc \n");
  printf(" B/b          -> Scale bond radius Up/Down\n");
  printf(" X/x          -> Scale atom radius Up/Down\n");
  printf(" U/u          -> Scale transparancy attribute for hydrid region Up/Down\n"); 
  printf(" V/v          -> Scale vector length Up/Down\n"); 
  printf(" T/t          -> Turn on/off display of species\n"); 
  printf(" Y/y          -> Toggle 3-D periodic boundry conditions\n");
  printf(" E/e          -> Pick/Show euler angles\n");
  printf("Key Pad Up    -> Translate up\n");
  printf("Key Pad Down  -> Translate down\n");
  printf("Key Pad Left  -> Translate left\n");
  printf("Key Pad Right -> Translate right\n");
  printf("Key Pad Pg Up -> Translate forward\n");
  printf("Key Pad Pg Up -> Translate back\n");
  printf("Alt_L + Right-click   -> Hide molecule at mouse position\n");
  printf("Shift_L + Right-click -> Change color of molecule at mouse position \n");
  printf("Alt_R + Right-click   -> hide atom at mouse position\n");
  printf("Shift_R + Right-click -> Change color of atom at mouse position\n");
  printf("\n");
}
/********************************************************************/
void quit_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
   exit(0);
}

void help_cb(Widget widget,XtPointer client_data,XtPointer call_data)
{
  printHelp();
}

/********************************************************************/
void load_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  static Widget dialog; /* make static for reuse */

  if(!dialog){
    dialog = XmCreateFileSelectionDialog(toplevel,"Attribute_Select",NULL,0);
    XtAddCallback(dialog,XmNokCallback,load_att,NULL);
    XtAddCallback(dialog,XmNcancelCallback,
		  (XtCallbackProc)XtUnmanageChild,NULL);
  }

  XtManageChild(dialog);
  XtPopup(XtParent(dialog),XtGrabNone);
}

void load_att(Widget w,XtPointer client_data,XtPointer call_data)
{
  int i;
  char *name = NULL;
  XmFileSelectionBoxCallbackStruct *cbs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  if(cbs){
    if(!XmStringGetLtoR (cbs->value,XmFONTLIST_DEFAULT_TAG,&name)) return;
    read_attributes(name,sys.natoms,sys.atoms);
    XtFree(name);
  }
  for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;

  switch(sv.movie_type){
  case 1: /* shelly */
  case 2: /* pdb */
    searchBonds(sys);
/*    search_hbonds(&sys); */
    break;
  case 0:  /* mike */
/*    search_hbonds(&sys); */
    break;
  }
    
  
  redraw();
}

/********************************************************************/
void vecl_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  static Widget dialog; /* make static for reuse */

  if(!dialog){
    dialog = XmCreateFileSelectionDialog(toplevel,"Vector_Select",NULL,0);
    XtAddCallback(dialog,XmNokCallback,load_vec,NULL);
    XtAddCallback(dialog,XmNcancelCallback,
		  (XtCallbackProc)XtUnmanageChild,NULL);
  }

  XtManageChild(dialog);
  XtPopup(XtParent(dialog),XtGrabNone);
}

void load_vec(Widget w,XtPointer client_data,XtPointer call_data)
{
  int i;
  char *name = NULL;
  XmFileSelectionBoxCallbackStruct *cbs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  if(cbs){
    if(!XmStringGetLtoR (cbs->value,XmFONTLIST_DEFAULT_TAG,&name)) return;
    readVectors(name,&(sys.nvectors),&(sys.vectors));
    XtFree(name);
  }
  sv.vectors=1;
  for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
  
  redraw();
}

/********************************************************************/
void vecla_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  static Widget dialog; /* make static for reuse */

  if(!dialog){
    dialog = XmCreateFileSelectionDialog(toplevel,"Vector_Select",NULL,0);
    XtAddCallback(dialog,XmNokCallback,load_veca,NULL);
    XtAddCallback(dialog,XmNcancelCallback,
		  (XtCallbackProc)XtUnmanageChild,NULL);
  }

  XtManageChild(dialog);
  XtPopup(XtParent(dialog),XtGrabNone);
}

void load_veca(Widget w,XtPointer client_data,XtPointer call_data)
{
  int i;
  char *name = NULL;
  XmFileSelectionBoxCallbackStruct *cbs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  if(cbs){
    if(!XmStringGetLtoR (cbs->value,XmFONTLIST_DEFAULT_TAG,&name)) return;
    {
      float **xv; /* obtain a vector of vectors for xconfig */
      xv = (float **)malloc(sys.nconf*sizeof(float *));
      for(int i=0;i<sys.nconf;++i) xv[i] = sys.coords[i].getx();
      readVectorsAtoms(name,&(sys.nvectors),&(sys.vectors),sys.natoms,xv);
      free(xv);
    }
    XtFree(name);
  }
  sv.vectors=1;
  for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
  
  redraw();
}

/*------------------------------------------------------------------*/
void elli_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  static Widget dialog; /* make static for reuse */

  if(!dialog){
    dialog = XmCreateFileSelectionDialog(toplevel,"Ellipse_Select",NULL,0);
    XtAddCallback(dialog,XmNokCallback,load_elli,NULL);
    XtAddCallback(dialog,XmNcancelCallback,
		  (XtCallbackProc)XtUnmanageChild,NULL);
  }

  XtManageChild(dialog);
  XtPopup(XtParent(dialog),XtGrabNone);
}

void load_elli(Widget w,XtPointer client_data,XtPointer call_data)
{
  char *name = NULL;
  XmFileSelectionBoxCallbackStruct *cbs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  if(cbs){
    if(!XmStringGetLtoR (cbs->value,XmFONTLIST_DEFAULT_TAG,&name)) return;
    printf("calling read_ellipse not updated!!!\n");
    exit(1);
    // read_ellipse(name,sys.natoms,sys.atoms);
    XtFree(name);
  }
  
  redraw();
}

/********************************************************************/
void save_movie_cb(Widget widget,XtPointer client_data,XtPointer call_data)
{
  FILE *fp;
  Dimension width,height;
  int i,j,k,w,h,npix,n,iframe;
  GLfloat *rbuf,*gbuf,*bbuf;
  char cr,cg,cb;
  char name[128];

  XtVaGetValues(da,XmNwidth,&width,XmNheight,&height,NULL);

  w=(int)width;
  h=(int)height;

  npix = w*h;
  rbuf = (GLfloat*)malloc(npix*sizeof(GLfloat));
  gbuf = (GLfloat*)malloc(npix*sizeof(GLfloat));
  bbuf = (GLfloat*)malloc(npix*sizeof(GLfloat));

  sv.frame=0;  

  for(iframe=0;iframe<sys.nconf;iframe++){

    redraw();
    glFlush();
    glXSwapBuffers(dpy,win); /* swap display buffer with frame buffer */
    
    glReadPixels(0,0,w,h,GL_RED,GL_FLOAT,rbuf);
    glReadPixels(0,0,w,h,GL_GREEN,GL_FLOAT,gbuf);
    glReadPixels(0,0,w,h,GL_BLUE,GL_FLOAT,bbuf);
    
    n=255;
    sprintf(name,"xmov%d.ppm",iframe);
    if((fp = fopen(name,"w"))==NULL){
      fprintf(stderr,"ERROR:can't open file %s\n",name);
      return;
    }
    printf("Saving %s (frame # %d)\n",name,iframe);

    fprintf(fp,"P6\n");
    fprintf(fp,"%d %d %d\n",w,h,n);
    for(i=h-1;i>=0;i--){
      for(j=0;j<w;j++){
	k = i*w+j;
	cr = (int)(rbuf[k]*n);
	cg = (int)(gbuf[k]*n);
	cb = (int)(bbuf[k]*n);
	
	fwrite(&cr,sizeof(char),1,fp);
	fwrite(&cg,sizeof(char),1,fp);
	fwrite(&cb,sizeof(char),1,fp);
      }
    }
    fclose(fp);

    show_movie((XtPointer)1);
  }

  glXSwapBuffers(dpy,win); /* swap display buffer with frame buffer */ 
  free(rbuf);free(gbuf);free(bbuf);
}

static void save_pos(int aframe,int natoms)
{
  FILE *fp;
  int i,j,k,nad;
  char name[128];
  float *px,vx[3];

  px = sys.coords[aframe].getx();
  
  printf("Enter file name for coordinates (%d.coord will be appended to it):",
     aframe);
  scanf("%s",name);
  sprintf(name,"%s%d.coord",name,aframe);
  if((fp = fopen(name,"w"))==NULL){
    fprintf(stderr,"ERROR:can't open file %s\n",name);
    return;
  }
  printf("Saving %s (frame # %d,natoms = %d)\n",name,aframe,natoms);

  nad = 0;
  for(i=0;i<natoms;++i){  /* loop over atoms */
    if(sys.atoms[i].ishow()) ++nad; /* are they displayed? */
  }
  fprintf(fp,"%d 1\n",nad);

  for(i=0;i<natoms;++i){  /* loop over atoms */
    if(sys.atoms[i].ishow()){ /* are they displayed? */
      j = i*DIM;
      for(k=0;k<DIM;k++){ /* rotate */
	vx[k]=0;
	for(int l=0;l<DIM;++l) vx[k] += px[j+l]*sv.rot_mat[k+l*4];
      }
      fprintf(fp,"%g %g %g\n",vx[0],vx[1],vx[2]); /* print */
    }
  }
  for(i=0;i<9;i+=3) 
    fprintf(fp,"%g %g %g\n",sys.cell[aframe][i],
	    sys.cell[aframe][i+1],sys.cell[aframe][i+2]);
  fclose(fp);
}

/********************************************************************/
void save_frame_cb(Widget widget,XtPointer client_data,XtPointer call_data)
{
  FILE *fp;
  Dimension width,height;
  int i,j,k,w,h,npix,n;
  GLfloat *rbuf,*gbuf,*bbuf;
  char cr,cg,cb;
  char name[128];
  int aframe,natoms;

  aframe = sv.frame;
  natoms = sys.natoms;
  
  save_pos(aframe,natoms);
  
  XtVaGetValues(da,XmNwidth,&width,XmNheight,&height,NULL);

  w=(int)width;
  h=(int)height;

  npix = w*h;
  rbuf = (GLfloat*)malloc(npix*sizeof(GLfloat));
  gbuf = (GLfloat*)malloc(npix*sizeof(GLfloat));
  bbuf = (GLfloat*)malloc(npix*sizeof(GLfloat));

  redraw();
  glFlush();
  glXSwapBuffers(dpy,win); /* swap display buffer with frame buffer */
  
  glReadPixels(0,0,w,h,GL_RED,GL_FLOAT,rbuf);
  glReadPixels(0,0,w,h,GL_GREEN,GL_FLOAT,gbuf);
  glReadPixels(0,0,w,h,GL_BLUE,GL_FLOAT,bbuf);
  
  n=255;
  aframe = sv.frame;
  
  printf("Enter file name (%d.ppm will be appended to it):",aframe);
  scanf("%s",name);

  sprintf(name,"%s%d.ppm",name,aframe);
  if((fp = fopen(name,"w"))==NULL){
    fprintf(stderr,"ERROR:can't open file %s\n",name);
    return;
  }
  printf("Saving %s (frame # %d)\n",name,aframe);
  
  fprintf(fp,"P6\n");
  fprintf(fp,"%d %d %d\n",w,h,n);
  for(i=h-1;i>=0;i--){
    for(j=0;j<w;j++){
      k = i*w+j;
      cr = (int)(rbuf[k]*n);
      cg = (int)(gbuf[k]*n);
      cb = (int)(bbuf[k]*n);
      
      fwrite(&cr,sizeof(char),1,fp);
      fwrite(&cg,sizeof(char),1,fp);
      fwrite(&cb,sizeof(char),1,fp);
    }
  }
  fclose(fp);

  glXSwapBuffers(dpy,win); /* swap display buffer with frame buffer */ 
  free(rbuf);free(gbuf);free(bbuf);
}
/********************************************************************/
void poly_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  static Widget dialog; /* make static for reuse */

  if(!dialog){
    dialog = XmCreateFileSelectionDialog(toplevel,"Polygon_Select",NULL,0);
    XtAddCallback(dialog,XmNokCallback,load_poly,NULL);
    XtAddCallback(dialog,XmNcancelCallback,
		  (XtCallbackProc)XtUnmanageChild,NULL);
  }

  XtManageChild(dialog);
  XtPopup(XtParent(dialog),XtGrabNone);
}

void load_poly(Widget w,XtPointer client_data,XtPointer call_data)
{
  int i;
  char *name = NULL;
  XmFileSelectionBoxCallbackStruct *cbs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  if(cbs){
    if(!XmStringGetLtoR (cbs->value,XmFONTLIST_DEFAULT_TAG,&name)) return;
    read_polygons(name,&(sys.npolygons),&(sys.polygons));
    XtFree(name);
  }
  sv.polygons=1;
  for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
  
  redraw();
}
/********************************************************************/
void poly_n_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  static Widget dialog; /* make static for reuse */

  if(!dialog){
    dialog = XmCreateFileSelectionDialog(toplevel,"Polygon_n_Select",NULL,0);
    XtAddCallback(dialog,XmNokCallback,load_n_poly,NULL);
    XtAddCallback(dialog,XmNcancelCallback,
       (XtCallbackProc)XtUnmanageChild,NULL);
  }
  
  XtManageChild(dialog);
  XtPopup(XtParent(dialog),XtGrabNone);
}

void load_n_poly(Widget w,XtPointer client_data,XtPointer call_data)
{
  int i;
  char *name = NULL;
  XmFileSelectionBoxCallbackStruct *cbs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  if(cbs){
    if(!XmStringGetLtoR (cbs->value,XmFONTLIST_DEFAULT_TAG,&name)) return;
    read_n_polygons(name,&(sys.npolygons),&(sys.polygons));
    XtFree(name);
  }
  sv.polygons=1;
  for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
  
  redraw();
}
/********************************************************************/

void ribbon_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  static Widget dialog; /* make static for reuse */

  if(!dialog){
    dialog = XmCreateFileSelectionDialog(toplevel,"Ribbon_Select",NULL,0);
    XtAddCallback(dialog,XmNokCallback,load_ribbon,NULL);
    XtAddCallback(dialog,XmNcancelCallback,
		  (XtCallbackProc)XtUnmanageChild,NULL);
  }

  XtManageChild(dialog);
  XtPopup(XtParent(dialog),XtGrabNone);
}

void load_ribbon(Widget w,XtPointer client_data,XtPointer call_data)
{
  int i;
  char *name = NULL;
  XmFileSelectionBoxCallbackStruct *cbs = 
    (XmFileSelectionBoxCallbackStruct *) call_data;

  if(cbs){
    if(!XmStringGetLtoR (cbs->value,XmFONTLIST_DEFAULT_TAG,&name)) return;
    read_ribbons(name,&(sys.nribbons),&(sys.ribbons));
    XtFree(name);
  }
  sv.ribbons=1;
  for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
  
  redraw();
}

