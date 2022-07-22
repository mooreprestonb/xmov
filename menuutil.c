/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#include <stdio.h>
#include <stdlib.h>

#include "menuutil.h"

/*****************************************************************************/

Widget BuildMenu(Widget parent,int menu_type,char *menu_title,
		 char menu_mnemonic,Boolean tear_off,MenuItem *pmi,int nmenuItems)
{
  Widget menu,cascade,optionmenu,widget;
  int i;
  XmString str;
  Boolean radiobox;
  
  cascade = NULL;
  optionmenu = NULL;

  if ( menu_type==XmMENU_PULLDOWN || menu_type==XmMENU_OPTION ) {
    menu = XmCreatePulldownMenu(parent,"_pulldown",NULL,0);
  } else if ( menu_type==XmMENU_POPUP ) {
    menu = XmCreatePopupMenu(parent,"_popup",NULL,0);
  } else {
    XtWarning("Invalid menu type passed to BuildMenu()");
    return NULL;
  }
  if ( tear_off ) {
    XtVaSetValues(menu,XmNtearOffModel,XmTEAR_OFF_ENABLED,NULL);
  }
  /*
   * If this is a pulldown menu, we must make a cascade button.
   */
  if ( menu_type==XmMENU_PULLDOWN ) {
    str = XmStringCreateLocalized(menu_title);
    cascade = XtVaCreateManagedWidget(menu_title,
				      xmCascadeButtonGadgetClass,parent,
				      XmNsubMenuId,menu,
				      XmNlabelString,str,
				      XmNmnemonic,menu_mnemonic,
				      NULL);
    XmStringFree(str);
    /*
     * Option menus are a special case, but fairly easy to handle.
     */
  } else if ( menu_type==XmMENU_OPTION ) {
    Arg args[5];
    int n=0;
    str = XmStringCreateLocalized(menu_title);
    XtSetArg(args[n],XmNsubMenuId,menu); n++;
    XtSetArg(args[n],XmNlabelString,str); n++;
    
    optionmenu = XmCreateOptionMenu(parent,menu_title,args,n);
    XmStringFree(str);
  }
  /*
   * Now add menu items.
   */
  radiobox = False;
  for ( i=0; i<nmenuItems; ++i ) {
    /*
     * if there are submenu items, we create the pull-right menu by
     * calling this function recursively
     */
    if ( pmi[i].subitems ) {
      if ( menu_type==XmMENU_OPTION ) {
	XtWarning("Can't build submenus from Option menu items.");
	continue;
      } else {
	pmi[i].w = widget = BuildMenu(menu,XmMENU_PULLDOWN,pmi[i].label,
				      pmi[i].mnemonic,tear_off,
				      pmi[i].subitems,pmi[i].nsubitems);
      }
    } else {
      pmi[i].w = widget = XtVaCreateManagedWidget(pmi[i].label,
						  *pmi[i].wclass,menu,
						  NULL);
      if ( pmi[i].wclass==&xmToggleButtonGadgetClass
	   && pmi[i].radiobutton ) {
	radiobox=True;
	XtVaSetValues(widget,
		      XmNindicatorType,XmONE_OF_MANY,
		      NULL);
      }
    }
    /*
     * any item may have a mnemonic, so check and set
     */
    if ( pmi[i].mnemonic ) {
      XtVaSetValues(widget,XmNmnemonic,pmi[i].mnemonic,NULL);
    }
    /*
     * any item may have an accelerator, except for cascade menus,
     * but this isn't checked here.
     */
    if ( pmi[i].accel ) {
      str = XmStringCreateLocalized(pmi[i].accel_text);
      XtVaSetValues(widget,
		    XmNaccelerator,pmi[i].accel,
		    XmNacceleratorText,str,
		    NULL);
      XmStringFree(str);
    }
    
    if ( pmi[i].cb ) {
      XtAddCallback(widget,
		    ( pmi[i].wclass==&xmToggleButtonWidgetClass ||
		      pmi[i].wclass==&xmToggleButtonGadgetClass ) ?
		    XmNvalueChangedCallback :
		    XmNactivateCallback,
		    pmi[i].cb,pmi[i].cb_data);
    }
  }
  if ( radiobox && menu_type!=XmMENU_OPTION ) {
    XtVaSetValues(menu,
		  XmNentryClass,xmToggleButtonGadgetClass,
		  XmNradioBehavior,True,
		  XmNradioAlwaysOne,True,
		  NULL);
  }
  /*
   * For popup menus, return the menu; for pulldown menus, return the
   * cascade button; for option menus, return the thing created with
   * XmCreateOptionMenu(), which is, in fact, neither.
   */
  switch ( menu_type ) {
  case XmMENU_POPUP:
    return ( menu );
  case XmMENU_PULLDOWN:
    return ( cascade );
  case XmMENU_OPTION:
    return ( optionmenu );
  default :
    fprintf(stderr,"ERROR: wrong menu type in menuutil.c\n");
    exit(1);
  }
  fprintf(stderr,"ERROR: wrong menu type in menuutil.c\n");
  exit(1);
  return NULL;
}

