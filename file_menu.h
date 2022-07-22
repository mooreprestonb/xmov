/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/


#ifndef FILE_MENU_H_INCLUDED

#include "typedefs.h"

extern void quit_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void help_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void save_frame_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void save_movie_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void load_att(Widget w,XtPointer client_data,XtPointer call_data);
extern void load_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void load_vec(Widget w,XtPointer client_data,XtPointer call_data);
extern void vecl_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void load_veca(Widget w,XtPointer client_data,XtPointer call_data);
extern void vecla_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void elli_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void load_elli(Widget w,XtPointer client_data,XtPointer call_data);

extern void load_poly(Widget w,XtPointer client_data,XtPointer call_data);
extern void poly_cb(Widget w,XtPointer client_data,XtPointer call_data);
extern void load_n_poly(Widget w,XtPointer client_data,XtPointer call_data);
extern void poly_n_cb(Widget w,XtPointer client_data,XtPointer call_data);

extern void load_ribbon(Widget w,XtPointer client_data,XtPointer call_data);
extern void ribbon_cb(Widget w,XtPointer client_data,XtPointer call_data);
#define FILE_MENU_H_INCLUDED
#endif
