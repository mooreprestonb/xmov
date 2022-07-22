/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef EVENT_FUNC_H_INCLUDED

#include "typedefs.h"

extern void test_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void view_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void view_bond_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void resize(Widget w,XtPointer client_data,XtPointer call_data);
extern void setbgcol(Widget w,XtPointer client_data,XtPointer call_data);
extern void movie_input(Widget w,XtPointer client_data,XtPointer call_data);
extern void map_state_changed(Widget w,XtPointer client_data,XEvent *event,
   Boolean *cont);
extern void pmotion(Widget w,XtPointer client_data,XEvent *,Boolean *cont);

extern Boolean show_movie(XtPointer client_data);

#define EVENT_FUNC_H_INCLUDED
#endif
