#include <GL/gl.h>		
#include <GL/glx.h>
#ifdef LINUX
// #include <GL/GLwMDrawA.h>
#else
// #include <X11/GLw/GLwMDrawA.h>
#endif

const int DEFWIDTH=500;
const int DEFHEIGHT=500;

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "xmotif.h"

#ifdef SGI
#include <X11/extensions/SGIStereo.h>
#endif

#include "menuutil.h"

#include "display.h"
#include "initValues.h++"
#include "lighting.h"
#include "read_things.h"
#include "event_func.h"
#include "file_menu.h"
#include "stereo.h"
#include "menu.h"

static int stereoAttrs[] = {GLX_STEREO,GLX_RGBA, GLX_DOUBLEBUFFER,
			    GLX_RED_SIZE, 1,GLX_GREEN_SIZE, 1,GLX_BLUE_SIZE, 1,
			    GLX_DEPTH_SIZE, 1,
#ifdef ACCUM
			    GLX_ACCUM_RED_SIZE, 1,
			    GLX_ACCUM_GREEN_SIZE, 1, GLX_ACCUM_BLUE_SIZE, 1, 
			    GLX_ACCUM_ALPHA_SIZE, 1,
			    /* GLX_SAMPLES_SGIS,4, */
#endif
			    None};

static int visualAttrs[] = {GLX_RGBA, GLX_DOUBLEBUFFER,
			    GLX_RED_SIZE, 1,GLX_GREEN_SIZE, 1,GLX_BLUE_SIZE, 1,
			    GLX_DEPTH_SIZE, 1,
/* for antialiasing uncomment these values (GET RID OF STEREO) */                            
                            /* GLX_ACCUM_RED_SIZE, 1,
			    GLX_ACCUM_GREEN_SIZE, 1, GLX_ACCUM_BLUE_SIZE, 1, 
			    GLX_ACCUM_ALPHA_SIZE, 1,*/
			    None};

void reshape(SETVALUES &setvalues,int w,int h);

void popup_cb(Widget menu_item, XtPointer client_data, XtPointer call_data);

extern SYSTEM sys;
extern DLIST dl;
extern Display *dpy;
extern Widget da,toplevel;
extern XtAppContext app;
extern Window win;

void clear_data_win(void)
{
  unsigned int i;
  for(i=0;i<XtNumber(textDatainfo);i++){ 
    XmTextSetString(labels_info[i],"n/a");
  }
}

void update_data_win(SETVALUES &sv)
{
  char buf[64];
  float theta,phi,psi;
  float sd1,d1,sd2,d2;

  /* Number of atoms */
  sprintf(buf,"%d",sys.natoms);
  XmTextSetString(labels_info[0],buf);

  /* Number of bonds */
  if(sv.movie_type==0){
    sprintf(buf,"%d",sys.bonds[0].nbonds);
  } else {
    sprintf(buf,"%d",sys.bonds[sv.frame].nbonds);
  }
  XmTextSetString(labels_info[1],buf);

  /* Frame number */
  sprintf(buf,"%d",sv.frame);
  XmTextSetString(labels_info[2],buf);

  /* Translation */
  sprintf(buf,"(%.3g, %.3g, %.3g)",sv.transx,sv.transy,sv.transz);
  XmTextSetString(labels_info[3],buf);

  /* Euler Angles */
  d1=fabs(sd1=sv.rot_mat[10]);
  theta=acos(d1>1? sd1/d1: sd1);
  if ( theta==0 || theta==M_PI ) {
    phi=atan2(sv.rot_mat[1],sv.rot_mat[0]);
    psi=0;
  } else {
    phi=atan2(sv.rot_mat[8],-sv.rot_mat[9]);
    d2=fabs(sd2=cos(phi)*sv.rot_mat[0]+sin(phi)*sv.rot_mat[1]);   
    psi=acos(d2>1? sd2/d2: sd2);
    if ( sv.rot_mat[2]<0 ) psi=2*M_PI-psi;
  }
  if ( phi<0 ) phi += 2*M_PI;
  sprintf(buf,"(%.3g, %.3g, %.3g)",phi*180/M_PI,theta*180/M_PI,psi*180/M_PI);
  XmTextSetString(labels_info[4],buf);

  /* Quality factor */
  sprintf(buf,"%d",sv.sphere_quality);
  XmTextSetString(labels_info[5],buf);

  /* Bond Radius */
  sprintf(buf,"%.3g",sv.bond_radius);
  XmTextSetString(labels_info[6],buf);

  /* Scale factor */
  sprintf(buf,"%.3g",sv.scalefactor);
  XmTextSetString(labels_info[7],buf);

  /* Light intensity */
  sprintf(buf,"%d",sv.light_intensity);
  XmTextSetString(labels_info[8],buf);
}
/*******************************************************************/
static void init_from_toggles(SETVALUES &sv)
{
  sv.icell        = XmToggleButtonGetState(view_items[0].w);
  sv.iperspective = XmToggleButtonGetState(view_items[1].w);
  sv.solid        =!XmToggleButtonGetState(view_items[2].w);
  sv.disp_list    = XmToggleButtonGetState(view_items[3].w);
  sv.iatom        =!XmToggleButtonGetState(view_items[4].w);
  sv.bonds        =!XmToggleButtonGetState(view_bond_items[0].w);
  sv.bond_cyl     = XmToggleButtonGetState(view_bond_items[2].w);
  
  /*
   * Make sure at least one button is set!
   */
  
  if ( sv.bonds ) {
    if ( sv.bond_cyl ) {
      XmToggleButtonSetState(view_bond_items[2].w,True,False);
    } else {
      XmToggleButtonSetState(view_bond_items[1].w,True,False);
    }
  }
}

/*******************************************************************/
void createWindows(int argc,char *argv[],SETVALUES &sv)
{
  int i,scrn;
  Widget mainwin,file_menu,help_menu,bond_menu;
  Widget menubar,view_menu,bgopt_menu,rotopt_menu,pane,gltop,glwin;
  Widget data_win;
  //Widget light_menu;
  XtWidgetGeometry geom;
  GLXContext glctxt;
  Colormap cmap;
  XVisualInfo *vi;
  XSetWindowAttributes attrs;
  //Popup Menu declarations
  Widget popup_menu = NULL;
  XmString exit1;
  XmString dmcol;
  XmString exit_acc1;
  XmString t_angle;
  XmString dist;
  XmString b_angle;
  XmString refresh;
  XmString help;
  XmString bkcolor;
  XmString cellcolor; 
  XmString togglegrad;
  XmString polycol;
 //End of popup menu related declarations

  XtSetLanguageProc(NULL,NULL,NULL);
  toplevel = XtVaAppInitialize(&app,"XMovie",NULL,0,&argc,argv,NULL,NULL);
  /*
   * Create the main xmovie control window with menus, etc.
   */
  mainwin = XtVaCreateManagedWidget("main_window",xmMainWindowWidgetClass,
				    toplevel,NULL);
  
  pane = XtVaCreateWidget("pane",xmPanedWindowWidgetClass,mainwin,NULL);
  menubar = XmCreateMenuBar(pane,"menubar",NULL,0);
  file_menu=BuildMenu(menubar,XmMENU_PULLDOWN,"File",'F',True,file_items,nfile_items);
  view_menu=BuildMenu(menubar,XmMENU_PULLDOWN,"View",'V',True,view_items,nview_items);
  bond_menu=BuildMenu(menubar,XmMENU_PULLDOWN,"Bond",'D',True,view_bond_items,nview_bond_items);
  //light_menu=BuildMenu(menubar,XmMENU_PULLDOWN,"Lighting",'L',True,light_items,nlight_items);
  help_menu  = BuildMenu(menubar,XmMENU_PULLDOWN,"Help",'H',True,help_items,nhelp_items);
  XtVaSetValues(menubar,XmNmenuHelpWidget,help_menu,NULL);
  XtManageChild(menubar);
  
  bgopt_menu = BuildMenu(pane,XmMENU_OPTION,"BackGround",'G',True,bgcolors,nbgcolors);
  XtManageChild(bgopt_menu);
  rotopt_menu = BuildMenu(pane,XmMENU_OPTION,"Rotation",'R',True,rotations,nrotations);
  XtManageChild(rotopt_menu);
  
  geom.request_mode = CWHeight;
  XtQueryGeometry(menubar,NULL,&geom);
  XtVaSetValues(menubar,XmNpaneMinimum,geom.height,
		XmNpaneMaximum,geom.height,NULL);
  XtQueryGeometry(bgopt_menu,NULL,&geom);
  XtVaSetValues(bgopt_menu,XmNpaneMinimum,geom.height,
		XmNpaneMaximum,geom.height,NULL);
  XtQueryGeometry(rotopt_menu,NULL,&geom);
  XtVaSetValues(rotopt_menu,XmNpaneMinimum,geom.height,
		XmNpaneMaximum,geom.height,NULL);
  
  /* create data window */
  data_win = XtVaCreateWidget("data_win",xmRowColumnWidgetClass,pane,
			      XmNpacking,XmPACK_COLUMN,XmNnumColumns,
			      XtNumber(textDatainfo),XmNisAligned,True,
			      XmNentryAlignment,XmALIGNMENT_END,
			      XmNorientation,XmHORIZONTAL,NULL);
  for(i=0;(unsigned)i<XtNumber(textDatainfo);i++){
    XtVaCreateManagedWidget(textDatainfo[i],xmLabelGadgetClass,data_win,NULL);
    labels_info[i] = XtVaCreateManagedWidget("data_info",xmTextWidgetClass,
					     data_win,XmNeditable,False,
					     XmNcursorPositionVisible,False,
					     XmNtraversalOn,False,
					     XmNcolumns,25,NULL);
  }
  clear_data_win();
  XtManageChild(data_win);

  XtManageChild(pane);

  /*
   * Create a top-level window for displaying the graphics
   */
  gltop = XtVaCreatePopupShell("xmovie: glmovie window",
     topLevelShellWidgetClass,toplevel,NULL);
  
  dpy = XtDisplay(gltop);
  scrn = XScreenNumberOfScreen(XtScreen(toplevel));

  /* need correct vi for antialiassing */
  if ((vi = glXChooseVisual(dpy, scrn, stereoAttrs)) != NULL) {
    /* initialize for use with a stereo capable visual */
    stereoInit(True,"/usr/gfx/setmon -n 1280x492_120s",
	       "/usr/gfx/setmon -n 72HZ");
    printf("Stereo Capable Visual\n");    
  } else if ((vi = glXChooseVisual(dpy, scrn, visualAttrs)) != NULL) {
    /* initialize for use without a stereo capable visual */
    stereoInit(False,"/usr/gfx/setmon -n STR_BOT",
	       "/usr/gfx/setmon -n 72HZ");    
    printf("Stereo incapable visual (will try to fudge)\n");
  } else {
    XtAppError(app,"could not get visual");
    exit(1);
  }
  //#define NOHWEXCEL
#ifdef NOHWEXCEL
  printf("No hardware rendering set... not asking for it\n");
  glctxt = glXCreateContext(dpy,vi,0,GL_FALSE);
#else
  glctxt = glXCreateContext(dpy,vi,0,GL_TRUE);
#endif
  if(glctxt==NULL){
    XtAppError(app,"can't create rendering context");
    exit(1);
  }

  if(glXIsDirect(dpy,glctxt)){
    printf("Found direct connection to gl Hardware... GOOD\n");
  } else {
    printf("Using X server for graphics (this could be slow)\n");
  }
  cmap = XCreateColormap(dpy,RootWindow(dpy,vi->screen),vi->visual,AllocNone);
  XtVaSetValues(gltop,XtNvisual,vi->visual,XtNdepth,vi->depth,
		XtNcolormap,cmap,NULL);
  XtAddEventHandler(toplevel,StructureNotifyMask,False,map_state_changed,NULL);

  glwin = XtVaCreateManagedWidget("glwin",xmMainWindowWidgetClass,gltop,XmNwidth,DEFWIDTH,XmNheight,DEFHEIGHT,NULL);
  da=XtVaCreateManagedWidget("gldw_area",xmDrawingAreaWidgetClass,glwin,NULL);

  //Popup menu related stuff
  exit1=XmStringCreateLocalized("Exit");
  exit_acc1=XmStringCreateLocalized("Ctrl+C");
  t_angle=XmStringCreateLocalized("Torisional Angle");
  dist=XmStringCreateLocalized("Distance");
  b_angle=XmStringCreateLocalized("Bond Angle");
  refresh=XmStringCreateLocalized("Refresh");
  help=XmStringCreateLocalized("Help");
  bkcolor=XmStringCreateLocalized("Background Color");
  cellcolor=XmStringCreateLocalized("Cell Color");
  togglegrad=XmStringCreateLocalized("Toggle Gradient");
  polycol=XmStringCreateLocalized("Density Map Color");
  dmcol=XmStringCreateLocalized("Density Map Bounds");
  
  /*  popup_menu=
    XmVaCreateSimplePopupMenu(da,"popup",popup_cb,
			      XmVaPUSHBUTTON, bkcolor,'K',NULL,NULL,
			      XmVaPUSHBUTTON, cellcolor,'C',NULL,NULL,
			      XmVaPUSHBUTTON, dist,'D',NULL,NULL,
			      XmVaPUSHBUTTON, b_angle,'B',NULL,NULL,
			      XmVaPUSHBUTTON, t_angle,'t',NULL, NULL,
			      XmVaPUSHBUTTON, refresh,'r',NULL,NULL,
			      XmVaPUSHBUTTON, help,'h',NULL,NULL,
			      XmVaPUSHBUTTON, togglegrad,'t',NULL,NULL,
			      XmVaPUSHBUTTON, polycol,'c', NULL, NULL,
			      XmVaPUSHBUTTON, dmcol,'c', NULL,NULL,
			      XmVaPUSHBUTTON, exit1, 'x',"Ctrl<Key>c",exit_acc1,
			      NULL);
  */
  XmStringFree(polycol);
  XmStringFree(exit1);
  XmStringFree(exit_acc1);
  XmStringFree(bkcolor);
  XmStringFree(dist);
  XmStringFree(b_angle);
  XmStringFree(t_angle);
  XmStringFree(refresh);
  XmStringFree(help);
  XmStringFree(cellcolor);
  XmStringFree(togglegrad);
  XmStringFree(dmcol);
  
  //End of popupmenu related stuff


  XtAddCallback(da,XmNexposeCallback,test_cb,NULL);
  XtAddCallback(da,XmNinputCallback,movie_input,popup_menu);
  XtAddCallback(da,XmNresizeCallback,resize,NULL);
  
  XtAddEventHandler(da,PointerMotionMask,False,pmotion,NULL);

  XtRealizeWidget(toplevel);
  XtPopup(XtParent(glwin),XtGrabNone);
  
  win = XtWindow(da);
  /*
   * change drawing area window so that it generates resize *and* expose events
   * even when resized smaller
   */
  attrs.bit_gravity = ForgetGravity;
  XChangeWindowAttributes(XtDisplay(da),XtWindow(da),CWBitGravity,&attrs);
  
  glXMakeCurrent(dpy,win,glctxt);
  stereoMakeCurrent(dpy,win,glctxt);

  /* initialize some properties for gl calls */
  initGL(sv);
  
  init_from_toggles(sv);
  readInput(sys,sv);
  //if (sv.iperspective ) reshape(sv,500,500);
  reshape(sv,500,500);
#ifdef TRANS
  /* Delay setting up user interaction translation until there
     is data to interact with. */
  if (trans == NULL) {
    trans = XtParseTranslationTable(glxareaTranslations);
    XtOverrideTranslations(da, trans);
  }
#endif
}

