/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef MENUUTIL_H_INCLUDED

#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>

typedef struct MENU_ITEM_T MenuItem;
struct MENU_ITEM_T 
{
   char *label;          /* label for the menu */
   WidgetClass *wclass;   /* pushbutton, label, separator, etc. */
   Boolean radiobutton;  /* set for togglebuttons to have radio behavior */
   char mnemonic;        /* NULL if no mnemonic */
   char *accel;          /* keyboard shortcut; NULL if none */
   char *accel_text;     /* to be made a "compound string" */
   void (*cb)(Widget,XtPointer,XtPointer); /* routine to call; NULL if none */
   XtPointer cb_data;    /* client data for callback */
   MenuItem *subitems;   /* pullright menu items; NULL if none */
   int nsubitems;         /* number of items in the pull right menu */
   Widget w;             /* the widget for this button */
};

extern Widget BuildMenu(Widget parent,int menu_type,char *menu_title,
			char mnemonic,Boolean tear_off,MenuItem *pmi,int nmenuitem);


#define MENUUTIL_H_INCLUDED
#endif
