
#include <GL/gl.h>
#include <GL/glx.h>
#ifdef LINUX
// #include <GL/GLwMDrawA.h>
#else
// #include <X11/GLw/GLwMDrawA.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "xmotif.h"
#include "typedefs.h"
#include "llist.h"

Widget colorwindow;/*The window that displays solid color*/
XColor color; /*The color in the colorwindow*/

/* extern char *_dtostr(double x); use atof()*/ 

extern Widget da;
extern SYSTEM sys;
extern SETVALUES sv;

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

double torsional_angle;
double bondangle;
double distance;

struct b_ranges
{
  int lb;
  int ub;
};

static int g_range_lb,g_range_ub,part;

extern void redraw(void);
 
NODE *head;

Widget PostDialog(Widget parent, int dialog_type, char *msg,char *title)
{
  Widget dialog;
  XmString text;
  dialog = XmCreateMessageDialog (parent, title, NULL, 0);
  text = XmStringCreateLocalized (msg);
  XtVaSetValues (dialog,
		 XmNdialogType, dialog_type,
		 XmNmessageString, text,
		 NULL);
  XmStringFree (text);
  XtManageChild (dialog);
  XtPopup (XtParent (dialog), XtGrabNone);
  return dialog;
}


Widget PostScrollText(Widget parent, char *msg, char *title)
{
  Arg args[4];
  Widget sctext;
  XtSetArg(args[0], XmNrows,      30);
  XtSetArg(args[1], XmNcolumns,   80);
  XtSetArg(args[2], XmNeditable,  False);
  XtSetArg(args[3], XmNeditMode,  XmMULTI_LINE_EDIT);
  sctext = XmCreateScrolledText( parent, title, args, 4 );
  XtManageChild(sctext);
  XmTextInsert( sctext, XmTextGetLastPosition( sctext ), msg );
  
  return sctext;
}

void new_value_dm1(Widget scale_w, XtPointer client_data, XtPointer call_data)
{
  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;
  g_range_lb=cbs->value;
  printf("g_range_lb=%d",g_range_lb);
  printf("%s: %d\n", XtName (scale_w), cbs->value);
}

void new_value_dm (Widget scale_w, XtPointer client_data, XtPointer call_data)
{
  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;
  g_range_ub=cbs->value;
  
  printf("%s: %d\n", XtName (scale_w), cbs->value); 
}

Widget create_scale_dm1 (Widget parent, char *title,
			 int max, int min, int value)
{
  Arg       args[8];
  int       n = 0;
  XmString  xms = XmStringCreateLocalized (title);
  Widget    scale;
  
  XtSetArg (args[n], XmNtitleString, xms); n++;
  XtSetArg (args[n], XmNmaximum, max);     n++;
  XtSetArg (args[n], XmNminimum, min);     n++;
  XtSetArg (args[n], XmNvalue, value);     n++;
  XtSetArg (args[n], XmNshowValue, True);  n++;
                                                                                
  scale = XmCreateScale (parent, title, args, n);
  XtAddCallback (scale, XmNvalueChangedCallback, new_value_dm1, NULL);
  XtManageChild (scale);
  return scale;
}

Widget create_scale_dm (Widget parent, char *title,
			int max, int min, int value)
{
  Arg       args[8];
  int       n = 0;
  XmString  xms = XmStringCreateLocalized (title);
  /* void      new_value(); callback for Scale widgets */
  Widget    scale;

  XtSetArg (args[n], XmNtitleString, xms); n++;
  XtSetArg (args[n], XmNmaximum, max);     n++;
  XtSetArg (args[n], XmNminimum, min);     n++;
  XtSetArg (args[n], XmNvalue, value);     n++;
  XtSetArg (args[n], XmNshowValue, True);  n++;

  scale = XmCreateScale (parent, title, args, n);
  XtAddCallback (scale, XmNvalueChangedCallback, new_value_dm, NULL);
  XtManageChild (scale);
  return scale;
}


Widget PostBounds(Widget parent, char *title)
{
  Widget rowcol,scale,scale1;
  Arg            args[2];
  XtSetArg (args[0], XmNorientation, XmHORIZONTAL); 
  rowcol = XmCreateRowColumn (parent, "rowcol", args, 1);
  scale1 = create_scale_dm1 (rowcol, "Lower Bound", sys.npolygons-1,0 ,0);
  scale = create_scale_dm (rowcol, "Upper Bound", sys.npolygons-1, 0,sys.npolygons-1);
  printf("Ranges set: %d %d \n",g_range_lb,g_range_ub);
  XtManageChild (rowcol);
  redraw();
  return rowcol;
}

void new_value (Widget scale_w, XtPointer client_data, XtPointer call_data)
{
  int i;
  int range1, range2;
  int rgb = (long int) client_data;
  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *) call_data;
  Colormap cmap = DefaultColormapOfScreen (XtScreen (scale_w));
  
  switch (rgb) {
  case DoRed   : color.red = (cbs->value << 8); break;
  case DoGreen : color.green = (cbs->value << 8); break;
  case DoBlue  : color.blue = (cbs->value << 8);
  }
  /* reuse the same color again and again */
  XFreeColors (XtDisplay (scale_w), cmap, &color.pixel, 1, 0);
  
  if (!XAllocColor (XtDisplay (scale_w), cmap, &color)) {
    puts ("Couldn't XAllocColor!"); exit(1);
  }
  
  XtVaSetValues (colorwindow, XmNbackground, color.pixel, NULL);
  
  switch(part) {
  case 0:
    sv.background[0]=color.red/65536.0;
    sv.background[1]=color.green/65536.0;
    sv.background[2]=color.blue/65536.0;
    sv.background[3]=0.0;
    glClearColor(color.red/65536.0,color.green/65536.0,color.blue/65536.0,0.0);     
    redraw();
    break;
  case 1:
    sv.c_col.red=color.red;
    sv.c_col.green=color.green;
    sv.c_col.blue=color.blue;
    redraw();
    break;
  case 2:    
    range1=g_range_lb;
    range2=g_range_ub;
    if(range2<sys.npolygons&&range1>=0)
      {
	for(i=range1;i<range2;i++)
	  {
	    sys.polygons.rcolor[i]=color.red/65536.0;
	    sys.polygons.gcolor[i]=color.green/65536.0;
	    sys.polygons.bcolor[i]=color.blue/65536.0;
	    sys.polygons.acolor[i]=0.6;
	  }
      }
    redraw();
    break;
    
  default:
    printf("Invalid choice!"); 
    break;
  }
}

Widget create_scale (Widget parent, char *name, int mask)
{
  Arg        args[8];
  int        n = 0;
  Widget     scale;
  XrmValue   from;
  XrmValue   to;
  XmString   xms = XmStringCreateLocalized (name);
  /* void       new_value(); */
  
  to.addr = NULL;
  from.addr = name;
  from.size = strlen (name) + 1;
  XtConvertAndStore (parent, XmRString, &from, XmRPixel, &to);
  
  XtSetArg (args[n], XmNforeground, (*(Pixel *) to.addr)); n++;
  XtSetArg (args[n], XmNtitleString, xms); n++;
  XtSetArg (args[n], XmNshowValue, True); n++;
  XtSetArg (args[n], XmNmaximum, 255); n++;
  XtSetArg (args[n], XmNscaleMultiple, 5); n++;
  scale = XmCreateScale (parent, name, args, n);
  XmStringFree (xms);
  
  XtAddCallback (scale, XmNdragCallback, new_value, (XtPointer) mask);
  XtAddCallback (scale, XmNvalueChangedCallback, new_value, 
		 (XtPointer) mask);
  XtManageChild (scale);
  
  return scale;
}

Widget PostColorMap(Widget parent,char *title)
{
  Widget rowcol,rc,scale;
  int n;
  Arg args[8];
  
  color.flags=DoRed|DoGreen|DoBlue;

  rowcol = XmCreateRowColumn (parent, "rowcol", NULL, 0);
  
  /* create a canvas for the color display */
  n = 0;
  XtSetArg (args[n], XmNheight, 100);             n++;
  XtSetArg (args[n], XmNbackground, color.pixel); n++;
  colorwindow = XmCreateDrawingArea (rowcol, "colorwindow", args, n);
  XtManageChild (colorwindow);
  
  /* create another RowColumn under the 1st */
  n = 0;
  XtSetArg (args[n], XmNorientation, XmHORIZONTAL); n++;
  rc = XmCreateRowColumn (rowcol, "rc", args, n);
  
  /* create the scale widgets */
  scale = create_scale (rc, "Red", DoRed);
  scale = create_scale (rc, "Green", DoGreen);
  scale = create_scale (rc, "Blue", DoBlue);
  
  XtManageChild (rc);
  XtManageChild (rowcol);
  
  return rowcol;
}

void popup_cb(Widget menu_item, XtPointer client_data, XtPointer call_data)
{   
  Widget infodialog,infodialog1;
  char *helptext;
  int item_no = (long int) client_data;
  char ascid[256];

  switch(item_no)
    {
    case EXIT: 
      printf("Exiting...");
      exit(0);
    case TANGLE: 
      sprintf(ascid,"%g",torsional_angle);
      infodialog=PostDialog(da,XmDIALOG_OK_BUTTON,ascid,"Torsional Angle in Degrees(Last Four Atoms-Clicked!)");
      break;
    case B_ANGLE:
      sprintf(ascid,"%g",bondangle);
      infodialog=PostDialog(da,XmDIALOG_OK_BUTTON,ascid,"Bond Angle in Degrees(Last Three Atoms-Clicked!)");
      break;
    case DIST:
      sprintf(ascid,"%g",distance);
      infodialog=PostDialog(da,XmDIALOG_OK_BUTTON,ascid,"Distance in Angstrom (Last Two Atoms-Clicked!)");
      
      break; 
    case BKCOLOR:
      part=0;  
      infodialog1=PostDialog(da,XmDIALOG_OK_BUTTON,NULL,"Change Background Color");
      infodialog=PostColorMap(infodialog1,"Change Background Color");
      break;
    case DMBOUND:
      infodialog1=PostDialog(da,XmDIALOG_OK_BUTTON,NULL,"Set Density Map Bounds");    
      infodialog=PostBounds(infodialog1,"Set Density Map Bounds");      
      break;
    case POLYCOL:
      part=2;
      infodialog1=PostDialog(da,XmDIALOG_OK_BUTTON,NULL,"Change Density Map Color");
      infodialog=PostColorMap(infodialog1,"Change Density Map  Color");
      /*
	if(g_range_ub<sys.npolygons&&g_range_lb>=0&&g_range_lb<=g_range_ub)
	{
	for(i=g_range_lb;i<g_range_ub;i++)
	{
	sys.polygons.rcolor[i]=color.red/65536.0;
	sys.polygons.gcolor[i]=color.green/65536.0;
	sys.polygons.bcolor[i]=color.blue/65536.0;
	sys.polygons.acolor[i]=0.6;
	}
	}
       	
	redraw();*/
      break; 
    case CELLCOLOR: 
      part=1;
      infodialog1=PostDialog(da,XmDIALOG_OK_BUTTON,NULL,"Change Cell Color");
      infodialog=PostColorMap(infodialog1,"Change Cell Color");
      break;                                                                   
    case TOGGLEGRAD:
      break; 
    case REFRESH:
      printf("out ref");
      while(head!=NULL)
	{
	  int found = head->found;
	  head=removenode(head);
	  sys.atoms[found].rcolor(sys.atoms[found].rcolor()-0.5);
	  sys.atoms[found].gcolor(sys.atoms[found].gcolor()-0.5);
	  sys.atoms[found].bcolor(sys.atoms[found].bcolor()-0.5);
	}
      break;
    case HELP:
      helptext="\n Key stroke   -> Action \n----------------------\nEscape Key   -> Quit\nSpace Bar    -> Start or stop movie\nArrow Keys   -> Rotate (about xy)\nPage Up/down -> Rotate (about z)\n1 or 2      -> Spin light source\nI/i         -> Increase/Decrease light intensity\nf/F         -> View next/previous frame\np           -> Toggle perspective\nc           -> Toggle viewing cell box\nQ/q         -> Increase/Decrease sphere and cylinder quality\nK/k         -> Increase/Decrease line width\nw           -> Toggle wire/solid objects\nl           -> Toggle cylinder lines\nZ/z         -> Scale up/down\na/A         -> Toggle periodic boundry conditions\nHome        -> Reset values\nr           -> Toggle rotation about fixed or body coordinates\ns           -> smooth with antialiasing (doesn't always work)\nB/b         -> Scale Bond radius Up/Down\nX/x         -> Scale Atom radius Up/Down\nV/v         -> Scale Vector length Up/Down\nT/t         -> Turn on/off display of species\nY/y         -> Toggle 3-D periodic boundry conditions\nE/e         -> Pick/Show euler angles\nKey Pad Up   -> Translate up  Key Pad Down -> Translate down\nKey Pad Left -> Translate left  Key Pad Right-> Translate right\nKey Pad Pg Up-> Translate forward  Key Pad Pg Up-> Translate back\nJ/j          -> Decrease/Increase cell line width	 \n";

      infodialog1=PostDialog(da,XmDIALOG_OK_BUTTON,NULL,"Help Window");
         
      infodialog=PostScrollText(infodialog1,helptext,"Help Window");
      break;
      
    default: 
      printf("No choice made");
      break;
    }    
  // exit (0);
  puts (XtName (menu_item)); /* Otherwise, just print the selection */
}


