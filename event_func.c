
#include <Xm/MainW.h>
#include <Xm/ScrolledW.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glx.h>

#include "menuutil.h"
#include "display.h"
#include "lighting.h"
#include "read_things.h"
#include "event_func.h"
#include "stereo.h"
#include "pick.h"
#include "glsphere.h"
#include "trackball.h"

#define NISOSURF 5

#define DMBOUND 9
#define POLYCOL 8
#define TOGGLEGRAD 7
#define HELP 6
#define EXIT 10
#define TANGLE 4
#define B_ANGLE 3
#define DIST 2
#define REFRESH 5
#define BKCOLOR 0
#define CELLCOLOR 1

extern Widget da;
extern Widget outtop;
extern XtAppContext app;
extern Display *dpy;
extern Window win;

extern SETVALUES sv;
extern SYSTEM sys;
extern DLIST dl;

int anti_display(SETVALUES &sv,SYSTEM *system,DLIST *dlist);
void reshape(SETVALUES &,int,int);

extern int anti_display(SETVALUES *psv,SYSTEM *system,DLIST *dlist);

extern MenuItem view_items[];
extern MenuItem view_bond_items[];

static float bgcols[][6] = { 
  { 0,0,0,1 },{ 1,1,1,1 },{ 1,0,0,1 },{ 0,1,0,1 },{ 0.1,0.1,0,1 },
  {0,0.2,0.4,1},{0,0.1,0.2,1},
};

static XtWorkProcId rot_workid,movie_workid;
static int rotating=0,movie_showing=0;

/********************************************************************/
void test_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
   redraw();
}

/********************************************************************/
void view_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  int i;
  Dimension width,height;
  int item=(long int) client_data;
  
  switch ( item ) {
  case 0:
    sv.icell = !sv.icell;
    break;
  case 1:
    sv.iperspective = !sv.iperspective;
    XtVaGetValues(da,XmNwidth,&width,XmNheight,&height,NULL);
    reshape(sv,width,height);
    break;
  case 2:
    sv.solid = !sv.solid;
    for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
    break;
  case 3:
    sv.disp_list = !sv.disp_list;
    break;
  case 4:
    sv.iatom = !sv.iatom;
    break;
  default:
    break;
  }
  redraw();
}

/********************************************************************/
void view_bond_cb(Widget w,XtPointer client_data,XtPointer call_data)
{
  int i;
  int item=(long int)client_data,nredraw=0;
  
  switch ( item ) {
  case 0:
    if ( sv.bonds ) {
      sv.bonds=0;
      nredraw=1;
    }
    break;
  case 1:
    if ( !sv.bonds ) {
      sv.bonds=1;
      nredraw=1;
    }
    if ( sv.bond_cyl ) {
      sv.bond_cyl=0;
      sv.vector_cyl=0;
      for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
      nredraw=1;
    }
    break;
  case 2:
    if ( !sv.bonds ) {
      sv.bonds=1;
      nredraw=1;
    }
    if ( !sv.bond_cyl ) {
      sv.bond_cyl=1;
      sv.vector_cyl=1;
      for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
      nredraw=1;
    }
    break;
  case 3:
    sv.hydro = !sv.hydro;
    nredraw=1;
    break;
  default:
    break;
  }
  if ( nredraw ) redraw();
}

/*------------------------------------------------------------------*/
static void set_offset(int iflag)
{
  float xoff[3];
  char line[MAXLINE];

  printf("Enter offset (xoff yoff zoff):");
  fgets(line,MAXLINE,stdin);
  sscanf(line,"%g %g %g",&xoff[0],&xoff[1],&xoff[2]);

  if(iflag) {
    for(int k=0;k<DIM;k++) sv.xoff[k] = xoff[k]; 
    printf("Moving viewport by xoff = %g, yoff = %g, zoff = %g\n",
	   xoff[0],xoff[1],xoff[2]);
  }
  else {
    printf("Moving atoms by xoff = %g, yoff = %g, zoff = %g\n",
	   xoff[0],xoff[1],xoff[2]);
    for(int k=0;k<DIM;k++) sv.xoff[k] = 0; /*+= xoff[k]; */
    int i,j,idot; 
    idot = sys.nconf/10; 
    for(j=0;j<sys.nconf;j++){ 
      float *px = (sys.coords[j]).getx(); 
      if(j==idot) { printf("."); fflush(stdout);}
      for(i=0;i<sys.natoms;i++)
	for(int k=0;k<DIM;k++) px[i*DIM+k] += xoff[k];
    }
    printf("done\n");
  }
  return;
}
/*------------------------------------------------------------------*/
void resize(Widget w,XtPointer client_data,XtPointer call_data)
{
  Dimension width,height;
  /* don't try OpenGL until window is realized! */
  if (XtIsRealized(w)) {
    XtVaGetValues(w,XmNwidth,&width,XmNheight,&height,NULL);
    reshape(sv,(int)width,(int)height);
    printf("width = %d  height = %d\n",width,height);
  }
}

/********************************************************************/
void setbgcol(Widget w,XtPointer client_data,XtPointer call_data)
{
   float *bgc;
   int icol = (long int) client_data;
   bgc = &(bgcols[icol][0]);
   sv.background[0] = bgc[0];
   sv.background[1] = bgc[1];
   sv.background[2] = bgc[2];
   sv.background[3] = bgc[3];
   glClearColor(bgc[0],bgc[1],bgc[2],bgc[3]);
   test_cb(w,NULL,NULL);
}

/*******************************************************************/
void map_state_changed(Widget w,XtPointer client_data,XEvent *event,Boolean *cont)
{
  switch ( event->type ) {
  case MapNotify:
    if ( movie_showing && movie_workid!=0 ) {
      movie_workid = XtAppAddWorkProc(app,show_movie,(XtPointer)1);
    }
    break;
  case UnmapNotify:
    if ( movie_showing ) XtRemoveWorkProc(movie_workid);
    break;
  }
}

/***************************************************************/
extern void movie_input(Widget w,XtPointer client_data,XtPointer call_data)
{
  XmDrawingAreaCallbackStruct *cbs=(XmDrawingAreaCallbackStruct *)call_data;
  Dimension width,height;
  XEvent *event = cbs->event;
  char buffer[1];
  KeySym keysym;
  int i,iredraw;

#ifdef DEBUG
  printf("event->type = %d\n",event->type);
#endif
  
  iredraw = 0;
  switch(event->type){
  case KeyPress:
    XLookupString((XKeyEvent *)event,buffer,1,&keysym,NULL);
#ifdef DEBUG
    printf("event = %x, keysym = %x\n",(int *)(event),(int *)keysym);
#endif
    switch ( keysym) {
    case XK_space:
      if ( !movie_showing ) {
	printf("Showing movie!\n");
	movie_workid = XtAppAddWorkProc(app,show_movie,(XtPointer)1);
	movie_showing = 1;
      } else {
	printf("Displaying single frame.\n");
	movie_showing=0;
	XtRemoveWorkProc(movie_workid);
      }
      break;
    case XK_Up:
      if ( !rotating ) {
	rot_workid = XtAppAddWorkProc(app,rotate,(void *)"xm");
	rotating=1;
      }
      break;
    case XK_Down:
      if ( !rotating ) {
	rot_workid = XtAppAddWorkProc(app,rotate,(void *)"xp");
	rotating=1;
      }
      break;
    case XK_Left:
      if ( !rotating ) {
	rot_workid = XtAppAddWorkProc(app,rotate,(void *)"ym");
	rotating=1;
      }
      break;
    case XK_Right:
      if ( !rotating ) {
	rot_workid = XtAppAddWorkProc(app,rotate,(void *)"yp");
	rotating=1;
      }
      break;
    case XK_Page_Up:
      if ( !rotating ) {
	rot_workid = XtAppAddWorkProc(app,rotate,(void *)"zp");
	rotating=1;
      }
      break;
    case XK_Page_Down:
      if ( !rotating ) {
	rot_workid = XtAppAddWorkProc(app,rotate,(void *)"zm");
	rotating=1;
      }
      break;
    case XK_1: 
      sv.light_spin = (sv.light_spin + 10) % 360;
      iredraw = 1;
      break;
    case XK_2: 
      sv.light_spin = (sv.light_spin - 10) % 360;
      iredraw = 1;
      break;
    case XK_I:
      sv.light_intensity++;
      if(sv.light_intensity>MAX_LIGHT) 
	sv.light_intensity=MAX_LIGHT;
      iredraw = 1;
      break;
    case XK_i:
      sv.light_intensity--;
      if(sv.light_intensity < 0) 
	sv.light_intensity = 0;
      iredraw = 1;
      break;
    case XK_f:
      show_movie((XtPointer)1);
      iredraw = 1;
      break;
    case XK_F:
      show_movie((XtPointer)-1);
      iredraw = 1;
      break;
    case XK_p: 
    case XK_P:
      XmToggleButtonSetState(view_items[1].w,!(sv.iperspective),True);
      iredraw = 1;
      break;
    case XK_c: 
    case XK_C:
      XmToggleButtonSetState(view_items[0].w,!(sv.icell),True);
      iredraw = 1;
      break;
    case XK_Q:
      sv.sphere_quality++;
      sv.cylinder_quality *= 2;
      glDeleteLists(1,1);
      glNewList(1,GL_COMPILE);
      sphere(sv.sphere_quality,sv.solid);
      glEndList();
      for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
      iredraw = 1;
      break;
    case XK_q:
      sv.sphere_quality--;
      if(sv.sphere_quality<0) sv.sphere_quality=0;
      sv.cylinder_quality/=2;
      if(sv.cylinder_quality<4) sv.cylinder_quality=4;
      glDeleteLists(1,1);
      glNewList(1,GL_COMPILE);
      sphere(sv.sphere_quality,sv.solid);
      glEndList();
      for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
      iredraw = 1;
      break;
    case XK_K:
      sv.line_width+=sv.line_granularity;
      if(sv.line_width>sv.line_range[1]) {
	sv.line_width-=sv.line_granularity;
	printf("line width = %g is max allowed \n",sv.line_range[1]);
      }
      iredraw = 1;
      break;
    case XK_k:
      sv.line_width-=sv.line_granularity;
      if(sv.line_width<sv.line_range[0]) {
	sv.line_width+=sv.line_granularity;
	printf("line width = %g is mim allowed \n",sv.line_range[0]);
      }
      iredraw = 1;
      break;
    case XK_X:
      sv.sphere_scale *= 1.1;
      for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
      iredraw = 1;
      break;
    case XK_x:
      sv.sphere_scale *= .9;
      for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
      iredraw = 1;
      break;
    case XK_w:
    case XK_W:
      XmToggleButtonSetState(view_items[2].w,sv.solid,True);
      glDeleteLists(1,1);
      glNewList(1,GL_COMPILE);
      sphere(sv.sphere_quality,sv.solid);
      glEndList();
      break;
    case XK_D:
    case XK_d:
      XmToggleButtonSetState(view_items[3].w,sv.solid,True);
      break;
    case XK_o:
    case XK_O:
      XmToggleButtonSetState(view_items[4].w,sv.solid,True);
      break;
    case XK_l:
    case XK_L:
      if ( sv.bonds ) {
	if ( sv.bond_cyl ) {
	  XmToggleButtonSetState(view_bond_items[0].w,True,True);
	} else {
	  XmToggleButtonSetState(view_bond_items[2].w,True,True);
	}
      } else {
	XmToggleButtonSetState(view_bond_items[1].w,True,True);
      }
      break;
    case XK_a:
      sv.iperd = !sv.iperd; sv.iperda = 0;
      printf("Periodic boundries (molecules) = %d\n",sv.iperd);
      iredraw = 1;
      break;
    case XK_A:
      sv.iperd = !sv.iperd; sv.iperda = 1;
      printf("Periodic boundries (atoms) = %d\n",sv.iperda);
      iredraw = 1;
      break;
    case XK_Z:
      sv.scalefactor *= 1.1;
      iredraw = 1;
      break;
    case XK_z:
      sv.scalefactor *= .9;
      iredraw = 1;
      break;
    case XK_Home:
      setValues(sv,sys);
      iredraw = 1;
      break;
    case XK_n:
      printf("BONDS OFF\n");
      bond_show(0);
      iredraw = 1;
      break;
    case XK_N:
      printf("BONDS ON\n");
      bond_show(1);
      iredraw = 1;
      break;
    case XK_b:
      sv.bond_radius *= .9;
      for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
      iredraw = 1;
      break;
    case XK_B:
      sv.bond_radius *= 1.1;
      for(i=0;i<sys.nconf;i++) sys.idisp[i] = 0;
      iredraw = 1;
      break;
    case XK_R:
    case XK_r:
      sv.rot_type = !sv.rot_type;
      iredraw = 1;
      break;
    case XK_y: /* set offset */
      set_offset(0);
      iredraw = 1;
      break;
    case XK_Y: 
      set_offset(1);
      iredraw = 1;
      break;
    case XK_KP_Right:
      sv.transx = sv.transx + .25 ;
      iredraw = 1;
      break;
    case XK_KP_Left:
      sv.transx = sv.transx - .25 ;
      iredraw = 1;
      break;
    case XK_KP_Up:
      sv.transy = sv.transy + .25 ;
      iredraw = 1;
      break;
    case XK_KP_Down:
      sv.transy = sv.transy - .25 ;
      iredraw = 1;
      break;
    case XK_KP_Page_Up:
      sv.transz = sv.transz + .25 ;
      iredraw = 1;
      break;
    case XK_KP_Page_Down:
      sv.transz = sv.transz - .25 ;
      iredraw = 1;
      break;
    case XK_u:
      sv.gamma -= .1;
      sv.gamma = (sv.gamma<0)?0:sv.gamma;
      iredraw = 1;
      break;
    case XK_U:
      sv.gamma += .1;
      sv.gamma = (sv.gamma>1)?1:sv.gamma;
      iredraw = 1;
      break;
    case XK_v:
      VECTS::scale *= .9;
      iredraw = 1;
      break;
    case XK_V:
      VECTS::scale *= 1.1;
      iredraw = 1;
      break;
    case XK_s:
      if (!sv.stereo.enabled) {stereoEnable();} 
      else {stereoDisable();}
      iredraw = 1;
      break;
    case XK_S:
      anti_display(sv,&sys,&dl);
      if(sv.antialias_off) sv.antialias_off = 0;
      else sv.antialias_off = 1;
      iredraw = 1;
      break;
    case XK_Return:
      if ( movie_showing ) {
	movie_showing=0;
	XtRemoveWorkProc(movie_workid);
      }
      break;
    case 207:
      printf("Hey you have 207 as well!\n");
      break;
    case XK_Shift_L:
      //fprintf(stdout,"You have pressed the left Shift Key\n");
      sv.berase = XK_Shift_L;
      break;
    case XK_Alt_L:
      //fprintf(stdout,"You have pressed the left Alt Key\n");
      sv.berase = XK_Alt_L;
      break;
    case XK_Shift_R:
      //fprintf(stdout,"You have pressed the right Shift Key\n");
      sv.berase = XK_Shift_R;
      break;
    case XK_Alt_R:
      //fprintf(stdout,"You have pressed the right Alt Key\n");
      sv.berase = XK_Alt_R;
      break;
    default:
      fprintf(stderr,"Key:\"%d\" pressed, which does nothing\n",(int)keysym);
      break;
    }
    break;
  case KeyRelease:
    XLookupString((XKeyEvent *)event,buffer,1,&keysym,NULL);
    switch ( keysym) {
    case XK_Up:
    case XK_Down:
    case XK_Left:
    case XK_Right:
    case XK_Page_Up:
    case XK_Page_Down:
      if ( rotating ) {
	rotating=0;
	XtRemoveWorkProc(rot_workid);
      }
      break;
    case XK_Escape:
      stereoDone();
      exit(0);
      iredraw = 1;
      break;
    case XK_E:
      euler_rot();
      iredraw = 1;
      break;
    case XK_e:
      euler();
      iredraw = 1;
      break;
    case XK_T: /* turn on all species */
      pic_show(1);
      iredraw = 1;
      break;
    case XK_t: /* turn off all species */
      pic_show(0);
      iredraw = 1;
      break;
    case XK_Shift_L:
    case XK_Alt_L:
    case XK_Shift_R:
    case XK_Alt_R:
      fprintf(stdout,"You have released an Shift or Alt Key\n");
      sv.berase = 0;
      break;
    default:
      iredraw = 0;
      break;
    }
    break;
  case ButtonPress:
    switch(event->xbutton.button){
    case Button1: /* trackball */
      sv.brot = 1;
      sv.xbegin = (int)event->xbutton.x;
      sv.ybegin = (int)event->xbutton.y;
      break;
    case Button2:  /* pick */
      XtVaGetValues(w,XmNwidth,&width,XmNheight,&height,NULL);
      pick(event->xbutton.x,event->xbutton.y,(int)width,(int)height,sv);
      break;
    case Button3:  /* pick */
      XtVaGetValues(w,XmNwidth,&width,XmNheight,&height,NULL);
      int iatom = pick(event->xbutton.x,event->xbutton.y,
		       (int)width,(int)height,sv);
      if(iatom!=-1){
	float fcolor[4];
	switch(sv.berase){
	case XK_Alt_L: /* remove atoms with */
	  showSpecMol(0,sys.atoms[iatom].ispec(),sys.atoms[iatom].imole());
	  break;
	case XK_Shift_L:
	  printf("enter RGB color for molecule %s(%d) of species %s(%d)\n",
		 sys.atoms[iatom].moleName(),sys.atoms[iatom].imole(),
		 sys.atoms[iatom].specName(),sys.atoms[iatom].ispec());
	  scanf("%f %f %f %f",&fcolor[0],&fcolor[1],&fcolor[2],&fcolor[3]);
	  colorSpecMol(sys.atoms[iatom].ispec(),sys.atoms[iatom].imole(),fcolor);
	  break;
	case XK_Alt_R:
	  showAtom(0,iatom);
	  break;
	case XK_Shift_R:
	  printf("enter RGB color for molecule %s(%d) of species %s(%d)\n",
		 sys.atoms[iatom].moleName(),sys.atoms[iatom].imole(),
		 sys.atoms[iatom].specName(),sys.atoms[iatom].ispec());
	  scanf("%f %f %f %f",&fcolor[0],&fcolor[1],&fcolor[2],&fcolor[3]);
	  sys.atoms[iatom].color4(fcolor);
	  break;
	}
	iredraw = 1;
      }
      break;
    } 
    break;
  case ButtonRelease:
    switch(event->xbutton.button){
    case Button1:
      sv.brot = 0;
      break;
    }
    break;
  }
  if(iredraw ==1) test_cb(w,NULL,NULL);
}

/****************************************************************/
Boolean show_movie(XtPointer client_data)
{
  int inc;
  int frame;
  inc = (long int) client_data;
  
  frame  = sv.frame;
  frame += inc;
  
  if ( frame==sys.nconf ) { sv.frame=0;} 
  else if (frame==-1){    sv.frame=sys.nconf;} 
  else {                  sv.frame=frame;}
  redraw();
  
  return False;
}

/****************************************************************/
void pmotion(Widget wid,XtPointer client_data,XEvent *event,Boolean *cont)
{
  int iredraw=0;
  float xi,yi,x,y,x1,x2,y1,y2,w,h;
  Dimension width,height;
  
  switch(event->type){
  case MotionNotify:
    if(sv.brot){
      xi = sv.xbegin;
      yi = sv.ybegin;
      x = event->xbutton.x;
      y = event->xbutton.y;
      
      XtVaGetValues(wid,XmNwidth,&width,XmNheight,&height,NULL);
      w = width;
      h = height;
      x1 = (2.*xi-w)/w;
      x2 = (2.*x-w)/w;
      y1 = -(2.*yi-h)/h;
      y2 = -(2.*y-h)/h;
      trackball(sv.aquat,x1,y1,x2,y2);
      add_quats(sv.aquat,sv.cquat,sv.cquat);
      build_rotmat(sv.rot_mat,sv.cquat);
      iredraw = 1;
      sv.xbegin = (int)x;
      sv.ybegin = (int)y;
    }
    break;
  }
  if(iredraw) redraw();
}
