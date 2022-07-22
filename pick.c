/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/
/* because the number of display names can be limited 
   we actually calculate the distance of each atom from
   an imaginary line on the screen (defined by the eye and
   the pointer */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glx.h>
/* #include <Xm/DrawingA.h> */
/* #include <X11/GLw/GLwMDrawA.h> */

#include "glsetvalues.h++"
#include "stereo.h"
#include "util_math.h"
#include "printinfo.h"

extern SYSTEM sys;
extern DLIST dl;

int pick(int x,int y,int wid,int h,SETVALUES &sv)
{
  static int if1=-1,if2,if3;
  int i,k,nconf,ifound;
  float tmp,v[DIM],p[DIM],w[DIM],u[DIM];
  float r2,rmat[9],rmati[9],zdis=0.;
  GLfloat fovy,fov2,aspect,near,width,height,left,right,top,bottom;
  //GLfloat far;

  if(if1 == -1) if2 = if3 = -1;
  width=(GLfloat) wid;
  height=(GLfloat) h;
  fovy = 40.;
  aspect = width/height;
  near = 1.;
  // GLfloat far=10.;
  
  fov2 = ((fovy*M_PI) / 180.0) / 2.0;
  top = near / (cos(fov2) / sin(fov2));
  bottom = -top;  
  right = top * aspect;
  left = -right;
  nconf = sv.frame % sys.nconf;

  rmat[0] = sv.rot_mat[0];
  rmat[1] = sv.rot_mat[1];
  rmat[2] = sv.rot_mat[2];
  rmat[3] = sv.rot_mat[4];
  rmat[4] = sv.rot_mat[5];
  rmat[5] = sv.rot_mat[6];
  rmat[6] = sv.rot_mat[8];
  rmat[7] = sv.rot_mat[9];
  rmat[8] = sv.rot_mat[10];

  /* calculate point in space of the x,y mouse position */
  p[0] = (x/(float)wid-.5)*(right-left);
  p[1] = (-y/(float)h+.5)*(top-bottom);
  p[2] = -near; 
  
  /* calculate where the eye is */
  if(sv.iperspective){ 
    for(k=0;k<DIM;++k) v[k] = 0.;
  } else {
    for(k=0;k<DIM;++k) v[k] = p[k];
    v[2] = 0.;
  }

  for(k=0;k<DIM;++k) v[k] = p[k] - v[k]; /* get directional vector */

  norm(DIM,v);

  /* go back to lab coordinates (order of ops is reversed!) */
  p[0] -= sv.transx;  p[1] -= sv.transy;  p[2] -= sv.transz;
  p[2] -= -3; /* hand scale so that the viewing volume 
	       is in front of the camera */

  scalevec(DIM,1./sv.scalefactor,p);

  getinv9(rmat,rmati); /* inverse matrix */

  /* rotate point and vector with view inverse of rotation matrix */
  for(k=0;k<DIM;++k)  w[k] = p[0]*rmati[k+0]+p[1]*rmati[k+3]+p[2]*rmati[k+6];

  for(k=0;k<DIM;++k) p[k] = w[k]; /* we now have the point in space */

  for(k=0;k<DIM;++k)  w[k] = v[0]*rmati[k+0]+v[1]*rmati[k+3]+v[2]*rmati[k+6];

  for(k=0;k<DIM;++k) v[k] = w[k]; /* this is the direction we are looking */
  
  /* final translation of the entire system */
  for(k=0;k<DIM;++k) p[k] += sv.xoff[k]; /* we now have the point in space */

#ifdef DEBUG
  printf("point = %g %g %g vector = %g %g %g\n",p[0],p[1],p[2],v[0],v[1],v[2]);
#endif

  /* now that we have a point in space and a direction vector
   * lets look along the vector for atoms with VDW radius that 
   * intersect the line
   */
  
  ifound = -1;
  float* xn = sys.coords[nconf].getx();
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].ishow()){   /* is that atom viewable */

      for(k=0;k<DIM;++k) w[k] = xn[i*DIM+k]-p[k];
      tmp = dot(DIM,w,v);
      for(k=0;k<DIM;++k) u[k] = tmp*v[k];

      /* make sure that the atom is not clipped out */
      if(dot(DIM,u,v) > 0.0){
	float u2 = dot(DIM,u,u);
        r2 = dot(DIM,w,w) - u2;

        tmp = sys.atoms[i].radius()*sv.sphere_scale;
        tmp = tmp*tmp;
        if(r2<tmp){
	  float du = sqrt(u2);
          float z = sqrt(tmp-r2);
          if(ifound==-1){
            ifound = i;
            zdis = du-z;
          } else { /* use closer one */
            if(zdis> du-z){
              ifound = i;
              zdis = du-z;
            }
          }
        }
      }
    }
  }

  if(ifound!= -1){   /* if we found an atom print stuff */
    printf("%d ", ifound);
    sys.atoms[ifound].printid(stdout);
    printf(", %g %g %g\n",xn[ifound*DIM],xn[ifound*DIM+1],xn[ifound*DIM+2]);
    if(if1 != -1){ /* get distance */
      printdist(if1,ifound,xn);
    }  
    if(if2 != -1){ /* get angle */
      printangle(if2,if1,ifound,xn);
    }
    if(if3 != -1){ /* get torsion */
      printtorsion(if3,if2,if1,ifound,xn);
    }
    if3 = if2;if2 = if1; if1 = ifound;
  } else {
    if1 = if2 = if3 = -1;
    printf("No atom there?\n");
  }
  return ifound;
}

void printShow(int ishow){
  if(ishow) printf("Showing ");
  else printf("Hiding ");
}

int showAll(int ishow){
  int i, icount = 0;
  printShow(ishow);
  printf("all atoms\n");
  for(i=0;i<sys.natoms;i++){
    sys.atoms[i].ishow(ishow);
    icount++;
  }
  return icount;
}

int showSpec(int ishow,int ispec){
  int i, icount = 0;
  printShow(ishow);
  printf("species %d\n",ispec);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].ispec() == ispec) {
      sys.atoms[i].ishow(ishow);
      icount++;
    }
  }
  return icount;
}

int showSpecType(int ishow,int ispec,int itype){
  int i,icount = 0;
  printShow(ishow);
  printf("species %d with type %d\n",ispec,itype);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].ispec() == ispec) {
      if(sys.atoms[i].itype() == itype) {
	sys.atoms[i].ishow(ishow);
	icount++;
      }
    }
  }
  return icount;
}

int showSpecAtomName(int ishow,int ispec,char *name){
  int i,icount = 0;
  printShow(ishow);
  printf("species %d with name \"%s\"\n",ispec,name);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].ispec() == ispec) {
      if(strcasecmp(sys.atoms[i].atomName(),name)==0){
	sys.atoms[i].ishow(ishow);
	icount++;               
      }
    }
  }
  return icount;
}

int showSpecMol(int ishow,int ispec, int imol){
  int i, icount= 0;
  printShow(ishow);
  printf("molecule %d of species %d\n", imol, ispec);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].ispec() == ispec) {
      if(sys.atoms[i].imole() == imol){
	sys.atoms[i].ishow(ishow);
	icount++;               
      }  
    }
  }
  return icount;
}

int colorSpecMol(int ispec, int imol, float color[4]){
  int i, icount= 0;
  printf("Coloring molecule %d of species %d to %f %f %f %f\n", 
	 imol, ispec , color[0], color[1], color[2], color[3]);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].ispec() == ispec) {
      if(sys.atoms[i].imole() == imol){
	sys.atoms[i].color4(color);
	icount++;               
      }  
    }
  }
  return icount;
}


int showSpecGroup(int ishow,int ispec,int igroup) {
  int i, icount = 0;
  printShow(ishow);
  printf("species %d with group %d\n",ispec,igroup);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].ispec() == ispec) {
      if(sys.atoms[i].igroup() == igroup) {
	sys.atoms[i].ishow(ishow);
	icount++;
      }
    }
  }
  return icount;
}

int showGroup(int ishow,int igroup) {
  int i, icount = 0;
  printShow(ishow);
  printf("group %d\n",igroup);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].igroup() == igroup) {
      sys.atoms[i].ishow(ishow);
      icount++;
    }
  }
  return icount;
}

int showGroupName(int ishow,char *name){
  int i, icount = 0;
  printShow(ishow);
  printf("group name \"%s\"\n",name);
  for(i=0;i<sys.natoms;i++){
    if(strcasecmp(sys.atoms[i].groupName(),name)==0){
      sys.atoms[i].ishow(ishow);
      icount++;               
    }
  }
  return icount;
}

int showSpecGroupName(int ishow,int ispec,char *name) {
  int i, icount = 0;
  printShow(ishow);
  printf("species %d with group name \"%s\"\n",ispec,name);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].ispec() == ispec) {
      if(strcasecmp(sys.atoms[i].groupName(),name)==0){
	sys.atoms[i].ishow(ishow);
	icount++;               
      }
    }
  }
  return icount;
}

int showAtom(int ishow,int ipick){
  printShow(ishow);
  printf("atom %d\n",ipick);
  sys.atoms[ipick].ishow(ishow);
  return 1;
}

int showType(int ishow,int itype){
  int i,icount = 0;
  printShow(ishow);
  printf("type %d\n",itype);
  for(i=0;i<sys.natoms;i++){
    if(sys.atoms[i].itype() == itype) {
      sys.atoms[i].ishow(ishow);
      icount++;
    }
  }
  return icount;
}

int showAtomName(int ishow,char *name){
  int i, icount = 0;
  printShow(ishow);
  printf("atoms with name \"%s\"\n",name);
  for(i=0;i<sys.natoms;i++){
    if(strcasecmp(sys.atoms[i].atomName(),name)==0){
      sys.atoms[i].ishow(ishow);
      icount++;               
    }
  }
  return icount;
}

/*------------------------------------------------------------------*/
void pic_show(int ishow)
{
  int i,ipick,icount,ispec,itype;
  char line[MAXLINE],name[MAXLINE],isact[MAXLINE];

  if(ishow) sprintf(isact,"Turning atoms on ");
  else sprintf(isact,"Turning atoms off ");
  printf("%s all(0), species (1), molecule (2), group(3), atom (4) :",isact);

  fgets(line,MAXLINE,stdin);
  sscanf(line,"%d",&ipick);
  icount = 0;
  switch(ipick){
  case 0: /* All */
    icount += showAll(ishow);
    break;
  case 2: /* molecule */
  case 1: /* species */
    ispec = -1;
    printf("Enter spec # or name to act on :");
    fgets(line,MAXLINE,stdin);
    if(line[0]>='0' && line[0] <='9') {
      sscanf(line,"%d",&ispec);
      printf("Looking for type %d\n",ispec);
      for(i=0;i<sys.natoms;i++){
	if(sys.atoms[i].ispec() == ispec){
	  strcpy(name,sys.atoms[i].moleName());
	  break;
	}
	if(i==sys.natoms){
	  printf("No such number %d %d\n",i,ispec);
	  return;
	}
      }
    } else {
      sscanf(line,"%s",name);
      printf("Looking for name \"%s\"\n",name);
      for(i=0;i<sys.natoms;i++){
	if(strcasecmp(sys.atoms[i].moleName(),name)==0){
	  ispec = sys.atoms[i].ispec();
	  break;
	}
      }
      if(i==sys.natoms) { printf("No such name\n");  return; }
    }
    if(ipick == 1){
      printf("%s spec \"%s\" (%d)\n",isact,name,ispec);       
      icount += showSpec(ishow,ispec);
    } else {
      printf("What do you want to do to \"%s\"\n",name);
      printf("act on atom (1), number (2) or group (3):");
      fgets(line,MAXLINE,stdin);
      sscanf(line,"%d",&ipick);         
      switch(ipick){
      case 1:
	printf("Enter atom type or name :");
	fgets(line,MAXLINE,stdin);
	if(line[0]>='0' && line[0] <='9') {
	  sscanf(line,"%d",&itype);
	  printf("Acting on itype %d on spec %s\n",
		 itype,sys.atoms[ispec].moleName());
	  icount += showSpecType(ishow,ispec,itype);
	} else {
	  sscanf(line,"%s",name);
	  printf("%s atoms with name \"%s\"\n",isact,name);
	  icount += showSpecAtomName(ishow,ispec,name);
	}
	break;
      case 2: 
	printf("%s, Enter number of molecule:",isact);
	fgets(line,MAXLINE,stdin);
	sscanf(line,"%d",&ipick);
	icount += showSpecMol(ishow,ispec,ipick);
	break;
      case 3:
	printf("Enter group # or name :");
	sscanf(line,"%s",name);
	if(line[0]>='0' && line[0] <='9') {
	  sscanf(line,"%d",&itype);
	  printf("Acting on group # %d\n",itype);
	  icount += showSpecGroup(ishow,ispec,itype);
	} else {
	  printf("%s groups with name \"%s\"\n",isact,name);
	  icount += showSpecGroupName(ishow,ispec,name);
	}
	break;
      default:
	printf("%d ***Not Implemented***\n",ipick);
	break;
      }
      break;
    }
    break;

  case 3: /* group */
    printf("Enter group -# for a number or name : ");
    fgets(line,MAXLINE,stdin);
    if(line[0]=='-'){
      sscanf(line,"%d",&itype); itype = -itype;
      printf("Acting on group # %d\n",itype);
      icount += showGroup(ishow,itype);
    } else {
      sscanf(line,"%s",name);
      printf("Acting on groups with name \"%s\"\n",name);
      icount += showGroupName(ishow,name);
    }
    break;

  case 4: /* atom */
    printf("Enter atom type or name or -# for specific atom :");
    fgets(line,MAXLINE,stdin);
    if(line[0]=='-'){
      sscanf(line,"%d",&ipick); ipick = -ipick;
      printf("%s %d (%s %s %s)\n",isact,ipick,sys.atoms[ipick].specName(),
	     sys.atoms[ipick].moleName(),sys.atoms[ipick].atomName());
      icount += showAtom(ishow,ipick);
      break;
    }
    if(line[0]>='0' && line[0] <='9') {
      sscanf(line,"%d",&itype);
      printf("%s, itype %d\n",isact,itype);
      icount += showType(ishow,itype);
    } else {
      sscanf(line,"%s",name);
      printf("%s atoms with name \"%s\"\n",isact,name);
      icount += showAtomName(ishow,name);
    }
    break;
  default :
    printf("%d not implemented\n",ipick);
    break;
  }
  
  printf("%s %d atoms\n",isact,icount);
  return;
}

int showAllbond(int ishow){
  int i,j, icount = 0;
  printShow(ishow);
  printf("all bonds\n");
  for(i=0;i<sys.nconf;i++){
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
    {
      (((sys.bonds[i]).bond[j]).ishow)(ishow);
      icount++;
    }
  }/*for sys.nconf*/
  return icount;
}

int showbondSpec(int ishow,int ispec){
  int i,j, icount = 0;
  printShow(ishow);
  printf("bond species %d\n",ispec);
  for(i=0;i<sys.nconf;i++){
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
    {
      if((sys.atoms[sys.bonds[i].bond[j].ib()].ispec() == ispec) || 
	 (sys.atoms[sys.bonds[i].bond[j].jb()].ispec() == ispec))
      {
	(((sys.bonds[i]).bond[j]).ishow)(ishow);
	icount++;
      }
    }
  }/*for sys.nconf*/
  return icount;
}

int showbondSpecType(int ishow,int ispec,int itype){
  int i,j,icount = 0;
  printShow(ishow);
  printf("species %d with type %d\n",ispec,itype);

  for(i=0;i<sys.nconf;i++){
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((sys.atoms[sys.bonds[i].bond[j].ib()].ispec() == ispec) ||
	   (sys.atoms[sys.bonds[i].bond[j].jb()].ispec() == ispec))
	  {
	    if((sys.atoms[sys.bonds[i].bond[j].ib()].itype() == itype) ||
	       (sys.atoms[sys.bonds[i].bond[j].jb()].itype() == itype))
	      {
		(((sys.bonds[i]).bond[j]).ishow)(ishow);
		icount++;
	      }/*end if*/
	  }/*end if*/
      }/*end for j*/
  }/*for i */
  return icount;
}

int showbondSpecAtomName(int ishow,int ispec,char *name){
  int i,j,icount = 0;
  printShow(ishow);
  printf("species %d with name \"%s\"\n",ispec,name);
  for(i=0;i<sys.nconf;i++){
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((sys.atoms[sys.bonds[i].bond[j].ib()].ispec() == ispec) ||
	   (sys.atoms[sys.bonds[i].bond[j].jb()].ispec() == ispec))
	{
	  if((strcasecmp(sys.atoms[sys.bonds[i].bond[j].ib()].atomName(),name)==0) ||
	     (strcasecmp(sys.atoms[sys.bonds[i].bond[j].jb()].atomName(),name)==0))
	  {
	    (((sys.bonds[i]).bond[j]).ishow)(ishow);
	    icount++;               
	  }/*end if*/
	}/*end if*/
      }/*end for j*/
  }/*end for i*/
  return icount;
}

int showbondSpecMol(int ishow,int ispec, int imol){
  int i,j, icount= 0;
  printShow(ishow);
  printf("molecule %d of species %d\n", imol, ispec);

  for(i=0;i<sys.nconf;i++){
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((sys.atoms[sys.bonds[i].bond[j].ib()].ispec() == ispec) || 
	   (sys.atoms[sys.bonds[i].bond[j].jb()].ispec() == ispec))
	  {
	    if((sys.atoms[sys.bonds[i].bond[j].ib()].imole() == imol) ||
	       (sys.atoms[sys.bonds[i].bond[j].jb()].imole() == imol))
	      {
		(((sys.bonds[i]).bond[j]).ishow)(ishow);
		icount++;               
	      }/*end if*/  
	  }/*end if*/
      }
  }
  return icount;
}

int showbondSpecGroupName(int ishow,int ispec,char *name) {
  int i,j, icount = 0;
  printShow(ishow);
  printf("species %d with group name \"%s\"\n",ispec,name);
  
  for(i=0;i<sys.nconf;i++){
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((sys.atoms[sys.bonds[i].bond[j].ib()].ispec() == ispec) || 
	   (sys.atoms[sys.bonds[i].bond[j].jb()].ispec() == ispec))
	  {
	    if((strcasecmp(sys.atoms[sys.bonds[i].bond[j].ib()].groupName(),name)==0) ||
	       (strcasecmp(sys.atoms[sys.bonds[i].bond[j].jb()].groupName(),name)==0))
	      {
		(((sys.bonds[i]).bond[j]).ishow)(ishow);
		icount++;               
	      }/*end if*/
	  }/*end if */
      }/*end for j*/
  }/*end for i*/
  return icount;
}

int showbondSpecGroup(int ishow,int ispec,int igroup) {
  int i,j, icount = 0;
  printShow(ishow);
  printf("species %d with group %d\n",ispec,igroup);
  for(i=0;i<sys.nconf;i++){
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((sys.atoms[sys.bonds[i].bond[j].ib()].ispec() == ispec) || 
	   (sys.atoms[sys.bonds[i].bond[j].jb()].ispec() == ispec))
	  {
	    if((sys.atoms[sys.bonds[i].bond[j].ib()].igroup() == igroup) ||
	       (sys.atoms[sys.bonds[i].bond[j].jb()].igroup() == igroup))
	    {
	      (((sys.bonds[i]).bond[j]).ishow)(ishow);
	      icount++;
	    }/*end if*/
	  }/*end if */
      }/*end for j*/
  }/*end for i*/
  return icount;
}

int showbondGroup(int ishow,int igroup) {
  int i,j, icount = 0;
  printShow(ishow);
  printf("group %d\n",igroup);
  for(i=0;i<sys.nconf;i++){
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((sys.atoms[sys.bonds[i].bond[j].ib()].igroup() == igroup) ||
	   (sys.atoms[sys.bonds[i].bond[j].jb()].igroup() == igroup))
	  {
	    (((sys.bonds[i]).bond[j]).ishow)(ishow);
	    icount++;
	  }/*end if*/
      }/*end for j*/
  }/*end for i*/
  return icount;
}

int showbondGroupName(int ishow,char *name){
  int i,j, icount = 0;
  printShow(ishow);
  printf("group name \"%s\"\n",name);
  for(i=0;i<sys.nconf;i++) {
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((strcasecmp(sys.atoms[sys.bonds[i].bond[j].ib()].groupName(),name)==0) ||
	   (strcasecmp(sys.atoms[sys.bonds[i].bond[j].jb()].groupName(),name)==0))
	  {
	    (((sys.bonds[i]).bond[j]).ishow)(ishow);
	    icount++;               
	  }/*end if*/
      }/*end for j*/
  }/*end for i*/
    return icount;
}

int showbondAtom(int ishow,int ipick){
  int i, j;
  printShow(ishow);
  printf("atom %d\n",ipick);

  for(i=0;i<sys.nconf;i++) {
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if(((sys.bonds[i].bond[j].ib()) == ipick) ||
	   ((sys.bonds[i].bond[j].jb()) == ipick))
	  {
	    (((sys.bonds[i]).bond[j]).ishow)(ishow);
	  }
      }/*end for j*/
  }/*end for i*/
  return 1;
}

int showbondType(int ishow,int itype){
  int i,j,icount = 0;
  printShow(ishow);
  printf("type %d\n",itype);
  for(i=0;i<sys.nconf;i++) {
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((sys.atoms[sys.bonds[i].bond[j].ib()].itype() == itype) ||
	   (sys.atoms[sys.bonds[i].bond[j].jb()].itype() == itype))
	  {
	    (((sys.bonds[i]).bond[j]).ishow)(ishow);
	    icount++;
	  }/*end if*/
      }/*end for j*/
  }/*end for i*/
  return icount;
}

int showbondAtomName(int ishow,char *name){
  int i,j, icount = 0;
  printShow(ishow);
  printf("atoms with name \"%s\"\n",name);
  for(i=0;i<sys.nconf;i++) {
    for(j = 0; j < sys.bonds[i].nbonds; ++j)
      {
	if((strcasecmp(sys.atoms[sys.bonds[i].bond[j].ib()].atomName(),name)==0) || 
	   (strcasecmp(sys.atoms[sys.bonds[i].bond[j].jb()].atomName(),name)==0))
	  {
	    (((sys.bonds[i]).bond[j]).ishow)(ishow);
	    icount++;               
	  }/*end if*/
      }/*end for j*/
  }/*end for i*/
  return icount;
}


void bond_show(int ishow)
{
  int i,ipick,icount,ispec,itype;
  char line[MAXLINE],name[MAXLINE],isact[MAXLINE];

  if(ishow) sprintf(isact,"Turning bonds on ");
  else sprintf(isact,"Turning bonds off ");
  printf("%s all(0), species (1), molecule (2), group(3), atom (4) :",isact);

  fgets(line,MAXLINE,stdin);
  sscanf(line,"%d",&ipick);
  icount = 0;
  switch(ipick){
  case 0: /* All */
    icount += showAllbond(ishow);
    break;
  case 2: 
  case 1: 
    ispec = -1;
    printf("Enter species # or name to  act on :");
    fgets(line,MAXLINE,stdin);
    if(line[0]>='0' && line[0] <='9') {
      sscanf(line,"%d",&ispec);
      printf("Looking for type %d\n",ispec);
      for(i=0;i<sys.natoms;i++){
	if(sys.atoms[i].ispec() == ispec){
	  strcpy(name,sys.atoms[i].moleName());
	  break;
	}
	if(i==sys.natoms){
	  printf("No such number %d %d\n",i,ispec);
	  return;
	}
      }
    } else {
      sscanf(line,"%s",name);
      printf("Looking for name \"%s\"\n",name);
      for(i=0;i<sys.natoms;i++){
	if(strcasecmp(sys.atoms[i].moleName(),name)==0){
	  ispec = sys.atoms[i].ispec();
	  break;
	}
      }
      if(i==sys.natoms) { printf("No such name\n");  return; }
    }
    if(ipick == 1){
      printf("%s spec \"%s\" (%d)\n",isact,name,ispec);       
      icount += showbondSpec(ishow,ispec);
    } else {
      printf("What do you want to do to \"%s\"\n",name);
      printf("act on atom (1), number (2) or group (3):");
      fgets(line,MAXLINE,stdin);
      sscanf(line,"%d",&ipick);         
      switch(ipick){
      case 1:
	printf("Enter atom type or name :");
	fgets(line,MAXLINE,stdin);
	if(line[0]>='0' && line[0] <='9') {
	  sscanf(line,"%d",&itype);
	  printf("Acting on itype %d on spec %s\n",
		 itype,sys.atoms[ispec].moleName());
	  icount += showbondSpecType(ishow,ispec,itype);
	} else {
	  sscanf(line,"%s",name);
	  printf("%s atoms with name \"%s\"\n",isact,name);
	  icount += showbondSpecAtomName(ishow,ispec,name);
	}
	break;
      case 2: 
	printf("%s, Enter number of molecule:",isact);
	fgets(line,MAXLINE,stdin);
	sscanf(line,"%d",&ipick);
	icount += showbondSpecMol(ishow,ispec,ipick);
	break;
      case 3:
	printf("Enter group # or name :");
	sscanf(line,"%s",name);
	if(line[0]>='0' && line[0] <='9') {
	  sscanf(line,"%d",&itype);
	  printf("Acting on group # %d\n",itype);
	  icount += showbondSpecGroup(ishow,ispec,itype);
	} else {
	  printf("%s groups with name \"%s\"\n",isact,name);
	  icount += showbondSpecGroupName(ishow,ispec,name);
	}
	break;
      default:
	printf("%d ***Not Implemented***\n",ipick);
	break;
      }
      break;
    }
    break;

  case 3: 
    printf("Enter group -# for a number or name : ");
    fgets(line,MAXLINE,stdin);
    if(line[0]=='-'){
      sscanf(line,"%d",&itype); itype = -itype;
      printf("Acting on group # %d\n",itype);
      icount += showbondGroup(ishow,itype);
    } else {
      sscanf(line,"%s",name);
      printf("Acting on groups with name \"%s\"\n",name);
      icount += showbondGroupName(ishow,name);
    }
    break;

  case 4: 
    printf("Enter atom type or name or -# for specific atom :");
    fgets(line,MAXLINE,stdin);
    if(line[0]=='-'){
      sscanf(line,"%d",&ipick); ipick = -ipick;
      printf("%s %d (%s %s %s)\n",isact,ipick,sys.atoms[ipick].specName(),
	     sys.atoms[ipick].moleName(),sys.atoms[ipick].atomName());
      icount += showbondAtom(ishow,ipick);
      break;
    }
    if(line[0]>='0' && line[0] <='9') {
      sscanf(line,"%d",&itype);
      printf("%s, itype %d\n",isact,itype);
      icount += showbondType(ishow,itype);
    } else {
      sscanf(line,"%s",name);
      printf("%s atoms with name \"%s\"\n",isact,name);
      icount += showbondAtomName(ishow,name);
    }
    break;
  default :
    printf("%d not implemented\n",ipick);
    break;
  }
  
  printf("%s %d bonds\n",isact,icount);
  return;
}

