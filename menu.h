// menu item
// we could use dynamic allocation of arrays ie [] and terminate with ,NULL,};
// instead of [16] but this a good consistancy check on the number of items
// and generates less warnings :-)

const int nfile_items = 17;
static MenuItem file_items[nfile_items] = {
  { "Open Group Code...",&xmPushButtonGadgetClass,False,'O', NULL,NULL,0,      0,NULL,0,NULL },
  { "Open JS...",     &xmPushButtonGadgetClass,False,'J', NULL,NULL,0,      0,NULL,0,NULL },
  { "sep1",           &xmSeparatorGadgetClass, False,(char)NULL,NULL,NULL,0,      0,NULL,0,NULL },
  { "Open Vector...",    &xmPushButtonGadgetClass,False,'V', NULL,NULL,vecl_cb,      0,NULL,0,NULL },
  { "Open Vector Atoms...",    &xmPushButtonGadgetClass,False,'A', NULL,NULL,vecla_cb,      0,NULL,0,NULL },
  { "Open Polygon...",    &xmPushButtonGadgetClass,False,'P', NULL,NULL,poly_cb,      0,NULL,0,NULL },
  { "Open Polygon w/ norm...",    &xmPushButtonGadgetClass,False,'P', NULL,NULL,poly_n_cb,      0,NULL,0,NULL },
  { "Open Ribbon...",    &xmPushButtonGadgetClass,False,'R', NULL,NULL,ribbon_cb,      0,NULL,0,NULL },   
  { "Open Ellipse...",    &xmPushButtonGadgetClass,False,'E', NULL,NULL,elli_cb,      0,NULL,0,NULL },
  { "sep2",           &xmSeparatorGadgetClass, False,(char)NULL,NULL,NULL,0,      0,NULL,0,NULL },
  { "Load Options...",&xmPushButtonGadgetClass,False,'O', NULL,NULL,0,      0,NULL,0,NULL },
  { "Load Attributes...",&xmPushButtonGadgetClass,False,'A', NULL,NULL,load_cb,      0,NULL,0,NULL },
  { "sep2",           &xmSeparatorGadgetClass, False,(char)NULL,NULL,NULL,0,      0,NULL,0,NULL },
  { "Save Frame",   &xmPushButtonGadgetClass,False,'F', NULL,NULL,save_frame_cb,0,NULL,0,NULL },
  { "Save Movie",   &xmPushButtonGadgetClass,False,'M', NULL,NULL,save_movie_cb,0,NULL,0,NULL },
  { "sep2",           &xmSeparatorGadgetClass, False,(char)NULL,NULL,NULL,0,      0,NULL,0,NULL },
  { "Quit",           &xmPushButtonGadgetClass,False,'Q', NULL,NULL,quit_cb,0,NULL,0,NULL },
};

const int nview_bond_items = 4;
MenuItem view_bond_items[nview_bond_items] = {
  { "Off",      &xmToggleButtonGadgetClass,True,'O',NULL,NULL,view_bond_cb,(XtPointer)0,NULL,0,NULL },
  { "Lines",    &xmToggleButtonGadgetClass,True,'L',NULL,NULL,view_bond_cb,(XtPointer)1,NULL,0,NULL },
  { "Cylinders",&xmToggleButtonGadgetClass,True,'C',NULL,NULL,view_bond_cb,(XtPointer)2,NULL,0,NULL },
  { "Hydrogen",&xmToggleButtonGadgetClass,False,'H',NULL,NULL,view_bond_cb,(XtPointer)3,NULL,0,NULL }
};

const int nview_items=5;
MenuItem view_items[5] = {
  { "Cell",       &xmToggleButtonGadgetClass,False,'C',NULL,NULL,view_cb,(XtPointer)0,NULL,0,NULL },
  { "Perspective",&xmToggleButtonGadgetClass,False,'P',NULL,NULL,view_cb,(XtPointer)1,NULL,0,NULL },
  { "Wireframe",  &xmToggleButtonGadgetClass,False,'W',NULL,NULL,view_cb,(XtPointer)2,NULL,0,NULL },
  { "Display List",  &xmToggleButtonGadgetClass,False,'D',NULL,NULL,view_cb,(XtPointer)3,NULL,0,NULL },
  { "Atoms",  &xmToggleButtonGadgetClass,False,'A',NULL,NULL,view_cb,(XtPointer)4,NULL,0,NULL }
  //{ "Bonds",      &xmPushButtonGadgetClass,  False,'B',NULL,NULL,0,NULL,view_bond_items,nview_bond_items,NULL },
};

const int nlight_items =0;
//static MenuItem light_items[0] = {  
// };

const int nhelp_items=1;
static MenuItem help_items[nhelp_items] = {
  {"Help",&xmPushButtonGadgetClass,False,'H',NULL,NULL,help_cb,0,NULL,0,NULL}
};

const int nbgcolors=7;
static MenuItem bgcolors[nbgcolors] = {
  { "Black",&xmPushButtonGadgetClass,False,'B',NULL,NULL,setbgcol,(XtPointer)0,NULL,0,NULL },
  { "White",&xmPushButtonGadgetClass,False,'W',NULL,NULL,setbgcol,(XtPointer)1,NULL,0,NULL },
  { "Red",  &xmPushButtonGadgetClass,False,'R',NULL,NULL,setbgcol,(XtPointer)2,NULL,0,NULL },
  { "Green",&xmPushButtonGadgetClass,False,'G',NULL,NULL,setbgcol,(XtPointer)3,NULL,0,NULL },
  { "Pukey",&xmPushButtonGadgetClass,False,'P',NULL,NULL,setbgcol,(XtPointer)4,NULL,0,NULL },
  { "IBlue",&xmPushButtonGadgetClass,False,'I',NULL,NULL,setbgcol,(XtPointer)5,NULL,0,NULL },
  { "DBlue",&xmPushButtonGadgetClass,False,'D',NULL,NULL,setbgcol,(XtPointer)6,NULL,0,NULL }
};

const int nrotations=4;
static MenuItem rotations[nrotations] = {
  { "x", &xmPushButtonGadgetClass,False,'\0',NULL,NULL,0,0,NULL,0,NULL },
  { "y", &xmPushButtonGadgetClass,False,'\0',NULL,NULL,0,0,NULL,0,NULL },
  { "z", &xmPushButtonGadgetClass,False,'\0',NULL,NULL,0,0,NULL,0,NULL },
  { "xy",&xmPushButtonGadgetClass,False,'\0',NULL,NULL,0,0,NULL,0,NULL }
};

char *textDatainfo[] = {"Number of Atoms:","Number of Bonds:","Frame :",
			"Translation:","Euler Angles:",
			"Quality :","Bond Radius :","Scale Factor:",
			"Light Intensity:"};

Widget labels_info[XtNumber(textDatainfo)];

/*
static char *glxareaTranslations =
  "#override\n<Btn1Down>:smrot()\n<Btn1Motion>:mrot()\n<Btn3Down>:pickn()\n";
  */
