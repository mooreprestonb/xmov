/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

/* file to read in attributes file */

#include <Xm/DrawingA.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "typedefs.h"
#include "read_things.h"
#include "get_mols.h"
#include "find_things.h"
#include "trackball.h"
#include "util_math.h"
#include "coords.h++"

// #define DEBUG

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

#define META_KEY_ATTRIBUTE "attribute"
const float RED    = 0.5;
const float GREEN  = 0.5;
const float BLUE   = 0.5;
const float RADIUS = 1.0;
const float VDW    = 2.0;
const float ALPHA  =  .8;
const int   ANUM   = -1;

//float POLYRCOLOR;
//float POLYGCOLOR;
//float POLYBCOLOR;
//float POLYALPHA; // transparency


// #define OFFSET /* if we want to move to center of box */


/* Numerical Recipes standard error handler */

void nrerror(char error_text[])
{
   fprintf(stderr,"Numerical Recipes run-time error...\n");
   fprintf(stderr,"%s\n",error_text);
   fprintf(stderr,"...now exiting to system...\n");
   exit(1);
}

#define NR_END 0

static float **fmatrix(int nrl, int nrh, int ncl, int nch)
/***********************************************************************/
/* allocate a float matrix with subscript range m[nrl..nrh][ncl..nch] */
{
   int i, nrow=nrh-nrl+1,ncol=nch-ncl+1;
   float **m;
 
   /* allocate pointers to rows */
   m=(float **) malloc((size_t)((nrow+NR_END)*sizeof(float*)));
   if (!m) nrerror("allocation failure 1 in fmatrix()");
   m += NR_END;
   m -= nrl;
 
   /* allocate rows and set pointers to them */
   m[nrl]=(float *) malloc((size_t)((nrow*ncol+NR_END)*sizeof(float)));
   if (!m[nrl]) nrerror("allocation failure 2 in fmatrix()");
   m[nrl] += NR_END;
   m[nrl] -= ncl;
 
   for(i=nrl+1;i<=nrh;i++) m[i]=m[i-1]+ncol;
 
   /* return pointer to array of pointers to rows */
   return m;
}

/***********************************************************************/
void setValues(SETVALUES &sv,SYSTEM &system)
{
  int i,j;
  float rbox,rmax;
  float xmin,xmax,ymin,ymax,zmin,zmax;

  sv.bond_radius = .1;
  sv.light_spin = 0;
  sv.light_intensity = 6;
  sv.show_movie = 0;
  sv.transx = sv.transy = sv.transz = 0;
  sv.antialias_off = 1;
  sv.iperd = sv.iperda = 0;


  /* find largest axis */
  rmax = 0.;
  /*
  c = system->cell;
  for(i=0;i<system->nconf;i++){
    rbox = c[i][0]*c[i][0]+c[i][1]*c[i][1]+c[i][2]*c[i][2];
    if(rmax < rbox) rmax = rbox;
    rbox = c[i][3]*c[i][3]+c[i][4]*c[i][4]+c[i][5]*c[i][5];
    if(rmax < rbox) rmax = rbox;
    rbox = c[i][6]*c[i][6]+c[i][7]*c[i][7]+c[i][8]*c[i][8];
    if(rmax < rbox) rmax = rbox;
  }
  */
  i = sv.frame;
  float* x = system.coords[i].getx();
  xmin = xmax = x[0];
  ymin = ymax = x[1];
  zmin = zmax = x[2];
  for(j=0;j<system.natoms;j++){
    xmin = MIN(xmin,x[j*DIM  ]); xmax = MAX(xmax,x[j*DIM  ]);
    xmin = MIN(xmin,x[j*DIM+1]); xmax = MAX(xmax,x[j*DIM+1]);
    xmin = MIN(xmin,x[j*DIM+2]); xmax = MAX(xmax,x[j*DIM+2]);
  }

#ifdef OFFSET
  sv.xoff[0] = (xmax+xmin)/2.;
  sv.xoff[1] = (ymax+ymin)/2.;
  sv.xoff[2] = (zmax+zmin)/2.;
  printf("xoff = %g %g %g\n",sv.xoff[0],sv.xoff[1],sv.xoff[2]);
#else
  sv.xoff[0] = sv.xoff[1] = sv.xoff[2] = 0.0;
#endif
  
  rbox = MAX((xmax-xmin),(ymax-ymin));
  rbox = MAX(rbox,(zmax-zmin));
  rmax = MAX(rmax,rbox);
  if(rmax == 0.) rmax = 1.;
  sv.scalefactor = 1./rmax;
  
  trackball(sv.cquat,0.,0.,0.,0.);
  sv.brot = 0;
  for(i=0;i<16;i++) sv.rot_mat[i] = 0.0;
  sv.rot_mat[0]=1.;sv.rot_mat[5]=1.;sv.rot_mat[10]=1.;sv.rot_mat[15]=1.;
  sv.rot_type = 0;
}

/**********************************************************************/
void read_attributes(char *filename,int natoms,ATOMS *atoms)
{
  static int natt=0;
  static ATTRIBUTES *att_root=NULL,*att;
  int iatom,i,ich,nline,nch,num_keys,icase;
  int num_dict=8,num_found[8];
  char line[MAXLINE];
  WORD dict[8],metakey;
  KEY key_root,*key_now;
  FILE *fp=NULL;
  
  strcpy(dict[0],"atom");      strcpy(dict[1],"red");
  strcpy(dict[2],"green");     strcpy(dict[3],"blue");
  strcpy(dict[4],"radius");    strcpy(dict[5],"vdw");
  strcpy(dict[6],"anum");      strcpy(dict[7],"alpha");

  att = att_root;

  if ( filename != NULL ) {
    /* open data base file */
    if ((fp = fopen(filename,"r")) == NULL){
      sprintf(line,"can't open attributes filename \"%s\"",filename);
      md_warning(line);
    } else {
      sprintf(line,"Searching data base %s for attributes",filename);
      md_stdout(line);
    }
  }

  /* assign default atom types */
  if(fp==NULL || filename == NULL){
    printf("Assigning default atom types\n");
    for(iatom=0;iatom<natoms;iatom++){
      search_def_att(atoms,iatom); 
    }
    return;
  }

  
  if(att==NULL){
    att_root = (ATTRIBUTES *)malloc(sizeof(ATTRIBUTES));
    att = att_root;
  } else {
    free_att(natt,att);
  }
  natt = 0;

  att->red = RED;
  att->green = GREEN;
  att->blue = BLUE;
  att->radius = RADIUS;
  att->vdw = VDW;
  att->anum = ANUM;
  att->alpha = ALPHA;
  att->next = NULL;

  nline = nch = 0;
  while( (ich = fgetc(fp)) != EOF ) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      get_meta_key(fp,filename,
		   line,&nch,&nline,metakey,&key_root,&num_keys);
      if(!strncasecmp(META_KEY_ATTRIBUTE,metakey,
         strlen(META_KEY_ATTRIBUTE))){
        
        for(i=0;i<num_dict;i++) num_found[i] = 0;
        for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
          icase = get_dict_num(num_dict,dict,key_now);
          if(icase == -1){
            print_dict(key_now->keyword,num_dict,dict);  exit(1);
          }
          num_found[icase]++;
          switch (icase) {
             case 0: strcpy(att->atom,key_now->keyarg); break;/* atom */
             case 1: att->red = atof(key_now->keyarg); break; /* red */
             case 2: att->green = atof(key_now->keyarg); break; /* green */
             case 3: att->blue = atof(key_now->keyarg); break; /* blue */
             case 4: att->radius = atof(key_now->keyarg); break; /* radius */
             case 5: att->vdw = atof(key_now->keyarg); break; /* vdw */
             case 6: att->anum = atoi(key_now->keyarg); break; /* anum */
             case 7: att->alpha = atof(key_now->keyarg); break; /* alpha */
             default:
               print_dict(key_now->keyword,num_dict,dict);
               exit(1); break;
          }
        }
        natt++;
        att->next = (ATTRIBUTES *)malloc(sizeof(ATTRIBUTES));
        att = att->next;
        
        strcpy(att->atom," ");
        att->red = RED;       att->green = GREEN; att->blue = BLUE;
        att->radius = RADIUS; att->vdw = VDW;     att->anum = ANUM;
        att->alpha = ALPHA;   att->next = NULL;
      }
      free_key(num_keys,&key_root);
    }
    if(ich == '\n'){nch = 0; nline++;}
  }

  /* assing atoms types */
  printf("Assigning attributes (atom colors, radii etc.. )\n");
  for(iatom=0;iatom<natoms;iatom++){
    att = att_root;
    for(i=0;i<natt;i++){
      if(!strcasecmp(att->atom,atoms[iatom].atomName())
         || att->anum == atoms[iatom].itype()){
        atoms[iatom].rcolor(att->red);
        atoms[iatom].gcolor(att->green);
        atoms[iatom].bcolor(att->blue);
        atoms[iatom].radius(att->radius);
        atoms[iatom].vdw(att->vdw);
        atoms[iatom].beta(att->alpha);
        break;
      }
      att = att->next;
    }
    if(i==natt){
      printf("Atom %d with type \"%s\" (itype = %d) ",iatom,
         atoms[iatom].atomName(),atoms[iatom].itype());
      printf("not found searching defaults\n");

      search_def_att(atoms,iatom);
    }
  }
}

/* routine to free ATTRIBUTES */
void free_att(int natt,ATTRIBUTES *att)
{
  int i;
  ATTRIBUTES *tmp;

  /* skip first one so we don't have to reallocate it */
  att = att->next;
  for(i=1;i<=natt;i++){
    tmp = att->next;
    free(att);
    att = tmp;
  }
}
/**********************************************************************/
void read_polygons(char *filename,int *npoly,POLYGONS *polygons)
{
  int i,j,npolygons,nconf,nsurf,count=0,count2=0,r=0;
  char line[MAXLINE],line3[MAXLINE],answer[1],*head[100],*p;
  float xn,yn,zn,POLYRCOLOR[50],POLYGCOLOR[50],POLYBCOLOR[50],POLYALPHA[50],bound[50];
  float POLYRCOLORtmp,POLYGCOLORtmp,POLYBCOLORtmp,POLYALPHAtmp;
  FILE *fp;
  
  /* open polygon file */
  if ( (fp = fopen(filename,"r")) == NULL){	/* "r" means for read only */
    sprintf(line,"can't open polygon filename \"%s\"",filename);
    md_warning(line);
    return;
  }
  
  /* read header */
  if(fgets(line,MAXLINE,fp)==NULL){
    sprintf(line,"can't get header of polygon file %s",filename);
    md_warning(line);
    return;
  }

/*########################################
# This is the check for the header line  #
########################################*/
  // Split the components of the header line into an array
  // (# npolygons1 npolygons2 npolygons3 ...)
  
  p=strtok(line," ");
  while(p!=NULL)
  {
	head[r] = p;
	++r;	// r is used to count the number of surfaces
	p=strtok(NULL," ");
  }
 
 
  count=0;
  // Reset file pointer
  rewind(fp);
  fgets(line3,MAXLINE,fp);
  sscanf("%s",line3);
  if(strcmp(head[0],"#") !=0){
    fprintf(stderr,"ERROR: something is wrong with the header of file %s\n",filename);
   return;
  }

nsurf = r-1;
npolygons=0;
for(i=1;i<r;i++)
{
	if(atoi(head[i]) != 0)
	{
		bound[i-1] = atoi(head[i]);
		npolygons += bound[i-1];
	}
	else{return;}
}
  printf("nsurfaces = %d npolygons = %d (remark: %s)\n",nsurf,npolygons,line3);
  
  
  /* count the configurations */
  nconf = 0;
  while(fgets(line3,MAXLINE,fp) != NULL){
    nconf++;
    for(i=1;i<npolygons;i++){
      fgets(line3,MAXLINE,fp);
    }
    fgets(line3,MAXLINE,fp);
  }
  /* reset the file pointer */
  rewind(fp);

  if(nconf==0){
    fprintf(stderr,"ERROR: Didn't find any configurations?!!!\n");
    exit(1);
  }
  fprintf(stdout,"Number of Polygon configs = %d\n",nconf);
  polygons->nconf = nconf;

  /* allocate memeory */
  polygons->x1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->y1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->z1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->x2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->y2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->z2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->x3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->y3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->z3 = fmatrix(0,nconf-1,0,npolygons-1);

  polygons->rcolor = (float *)malloc(npolygons*sizeof(float));
  polygons->gcolor = (float *)malloc(npolygons*sizeof(float));
  polygons->bcolor = (float *)malloc(npolygons*sizeof(float));
  polygons->acolor = (float *)malloc(npolygons*sizeof(float));

  polygons->indx = (DepthIndex *)malloc(npolygons*sizeof(DepthIndex));
 
  
  /* Here the user is going to be asked to customize colors.
     If the user wants to, he/she can enter the RGB and alpha
     values that they desire for each surface.  If they answer
     no, random values for RGB will be used and a value of 
     0.8 for alpha will be used.
  */
  printf("Custom Colors? Enter y or n\n");
  scanf("%s",answer);
  while((strcmp(answer,"y")!=0) && (strcmp(answer,"n")!=0))
  { printf("Enter y or n\n"); scanf("%s",answer); }
  if(strcmp(answer,"y")==0)
  {
  	while(count<nsurf)
	{
		printf("Enter R value for surface %d :\n",(count+1));
		scanf("%g",&POLYRCOLORtmp);
		POLYRCOLOR[count] = POLYRCOLORtmp;
		printf("Enter G value for surface %d :\n",(count+1));
		scanf("%g",&POLYGCOLORtmp);
		POLYGCOLOR[count] = POLYGCOLORtmp;
		printf("Enter B value for surface %d :\n",(count+1));
		scanf("%g",&POLYBCOLORtmp);
		POLYBCOLOR[count] = POLYBCOLORtmp;
		printf("Enter transparency for surface %d :\n",(count+1));
		scanf("%g",&POLYALPHAtmp);
		POLYALPHA[count] = POLYALPHAtmp;
		++count;
		
		
	}
  }
  else{fprintf(stdout,"Using random surface colors.\n");
  	while(count<nsurf){
  	POLYRCOLORtmp = rand()/((float)RAND_MAX + 1);
  	POLYRCOLOR[count] = POLYRCOLORtmp;
  	POLYGCOLORtmp = rand()/((float)RAND_MAX + 1);
  	POLYGCOLOR[count] = POLYGCOLORtmp;
  	POLYBCOLORtmp = rand()/((float)RAND_MAX + 1);
  	POLYBCOLOR[count] = POLYBCOLORtmp;
  	POLYALPHAtmp = 0.8;//rand()/((float)RAND_MAX + 1);
  	POLYALPHA[count] = POLYALPHAtmp;
  	++count;
  	}  
  }
  
count2=0;
r=0;
for(i=0;i<npolygons;i++)
{
	if(i<(count2+bound[r]))
	{
		//printf("%d\n",POLYRCOLOR[r]);
		polygons->rcolor[i] = POLYRCOLOR[r];
		polygons->gcolor[i] = POLYGCOLOR[r];
		polygons->bcolor[i] = POLYBCOLOR[r];
		polygons->acolor[i] = POLYALPHA[r]; // transparency
	}
	else if(r<nsurf)
	{
		count2+=bound[r];
		++r;
		      
	}
}


/*#### Here x,y,z values are read in ####*/
  for(j=0;j<nconf;j++){
    fgets(line3,MAXLINE,fp); /* skip over header */
    for(i=0;i<npolygons;i++){
      fscanf(fp,"%g %g %g %g %g %g %g %g %g",
         &(polygons->x1[j][i]),&(polygons->y1[j][i]),
         &(polygons->z1[j][i]),&(polygons->x2[j][i]),
         &(polygons->y2[j][i]),&(polygons->z2[j][i]),
         &(polygons->x3[j][i]),&(polygons->y3[j][i]),
         &(polygons->z3[j][i]));
    }
    fgets(line3,MAXLINE,fp); /* read to end of line */
  }
  fclose(fp);

/*#ifdef DEBUG
  for(j=0;j<nconf;j++){
    for(i=0;i<npolygons;i++){
      printf("%d %d %9g %9g %9g %9g %9g %9g %9g %9g %9g\n",j,i,
         polygons->x1[j][i],polygons->y1[j][i],polygons->z1[j][i],
         polygons->x2[j][i],polygons->y2[j][i],polygons->z2[j][i],
         polygons->x3[j][i],polygons->y3[j][i],polygons->z3[j][i]);
    }
  }
#endif*/
  polygons->nx1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->ny1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nz1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nx2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->ny2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nz2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nx3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->ny3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nz3 = fmatrix(0,nconf-1,0,npolygons-1);

  printf("getting normals\n");
  
  for(j=0;j<nconf;j++){
    for(i=0;i<npolygons;i++){
      get_normal(polygons->x1[j][i],polygons->y1[j][i],polygons->z1[j][i],
         polygons->x2[j][i],polygons->y2[j][i],polygons->z2[j][i],
         polygons->x3[j][i],polygons->y3[j][i],polygons->z3[j][i],
         &xn,&yn,&zn);
      polygons->nx1[j][i] = xn;
      polygons->ny1[j][i] = yn;
      polygons->nz1[j][i] = zn;
      polygons->nx2[j][i] = xn;
      polygons->ny2[j][i] = yn;
      polygons->nz2[j][i] = zn;
      polygons->nx3[j][i] = xn;
      polygons->ny3[j][i] = yn;
      polygons->nz3[j][i] = zn;
    }
  }
  
  *npoly = npolygons;
}


/**********************************************************************/
void read_n_polygons(char *filename,int *npoly,POLYGONS *polygons)
{

  FILE *fp,*fp2;
  int i,j,npolygons,nconf,nsurf,count=0,count2=0,r=0;
  char line[MAXLINE],line3[MAXLINE],answer[1],*head[100],*p;
  float POLYRCOLOR[50],POLYGCOLOR[50],POLYBCOLOR[50],POLYALPHA[50],bound[50];
  float POLYRCOLORtmp,POLYGCOLORtmp,POLYBCOLORtmp,POLYALPHAtmp;
  /* open polygon file */
  if ( (fp = fopen(filename,"r")) == NULL){
    sprintf(line,"can't open polygon filename \"%s\"",filename);
    md_warning(line);
    return;
  }
  
  strcat(filename,"_n");
  /* open polygon file */
  if ( (fp2 = fopen(filename,"r")) == NULL){
    sprintf(line,"can't open polygon normal filename \"%s\"",filename);
    md_warning(line);
    return;
  }

  /* read header */
  if(fgets(line,MAXLINE,fp)==NULL){
    sprintf(line,"can't get header of polygon file %s",filename);
    md_warning(line);
    return;
  }

  p=strtok(line," ");
  while(p!=NULL)
  {
	head[r] = p;
	++r;
	p=strtok(NULL," ");
  }

 
  count=0;
  rewind(fp);
  fgets(line3,MAXLINE,fp);
  sscanf("%s",line3);
  if(strcmp(head[0],"#") !=0){
    fprintf(stderr,"ERROR: something is wrong with the header of file %s\n",filename);
   return;
  }
 
 

   nsurf = r-1;
   npolygons=0;
   for(i=1;i<r;i++)
   {
	if(atoi(head[i]) != 0)
	{
		bound[i-1] = atoi(head[i]);
		npolygons += bound[i-1];
	}
	else{return;}
   }



  printf("nsurfaces = %d npolygons with normals = %d (remark: %s)\n",nsurf,npolygons,line3);
  /* count the configurations */
  nconf = 0;
  while(fgets(line3,MAXLINE,fp) != NULL){
    nconf++;
    for(i=1;i<npolygons;i++){
      fgets(line3,MAXLINE,fp);
    }
    fgets(line3,MAXLINE,fp);
  }
  /* reset the file pointers */
  rewind(fp);
  rewind(fp2);

  if(nconf==0){
    fprintf(stderr,"ERROR: Didn't find any configurations?!!!\n");
    exit(1);
  }
  fprintf(stdout,"Number of Polygon configs = %d\n",nconf);
  polygons->nconf = nconf;

  /* allocate memeory */
  polygons->x1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->y1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->z1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->x2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->y2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->z2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->x3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->y3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->z3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nx1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->ny1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nz1 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nx2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->ny2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nz2 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nx3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->ny3 = fmatrix(0,nconf-1,0,npolygons-1);
  polygons->nz3 = fmatrix(0,nconf-1,0,npolygons-1);

  polygons->rcolor = (float *)malloc(npolygons*sizeof(float));
  polygons->gcolor = (float *)malloc(npolygons*sizeof(float));
  polygons->bcolor = (float *)malloc(npolygons*sizeof(float));
  polygons->acolor = (float *)malloc(npolygons*sizeof(float));

  polygons->indx = (DepthIndex *)malloc(npolygons*sizeof(DepthIndex));

printf("Custom Colors? Enter y or n\n");
  scanf("%s",answer);
  while((strcmp(answer,"y")!=0) && (strcmp(answer,"n")!=0))
  { printf("Enter y or n\n"); scanf("%s",answer); }
  if(strcmp(answer,"y")==0)
  {
  	while(count<nsurf)
	{
		printf("Enter R value for surface %d :\n",(count+1));
		scanf("%g",&POLYRCOLORtmp);
		POLYRCOLOR[count] = POLYRCOLORtmp;
		printf("Enter G value for surface %d :\n",(count+1));
		scanf("%g",&POLYGCOLORtmp);
		POLYGCOLOR[count] = POLYGCOLORtmp;
		printf("Enter B value for surface %d :\n",(count+1));
		scanf("%g",&POLYBCOLORtmp);
		POLYBCOLOR[count] = POLYBCOLORtmp;
		printf("Enter transparency for surface %d :\n",(count+1));
		scanf("%g",&POLYALPHAtmp);
		POLYALPHA[count] = POLYALPHAtmp;
		++count;
		
		
	}
  }
  else{fprintf(stdout,"Using random surface colors.\n");
  	while(count<nsurf){
  	POLYRCOLORtmp = rand()/((float)RAND_MAX + 1);
  	POLYRCOLOR[count] = POLYRCOLORtmp;
  	POLYGCOLORtmp = rand()/((float)RAND_MAX + 1);
  	POLYGCOLOR[count] = POLYGCOLORtmp;
  	POLYBCOLORtmp = rand()/((float)RAND_MAX + 1);
  	POLYBCOLOR[count] = POLYBCOLORtmp;
  	POLYALPHAtmp = 0.8;//rand()/((float)RAND_MAX + 1);
  	POLYALPHA[count] = POLYALPHAtmp;
  	++count;
  	}  
  }
count2=0;
r=0;
for(i=0;i<npolygons;i++)
{
	if(i<(count2+bound[r]))
	{
		//printf("%d\n",POLYRCOLOR[r]);
		polygons->rcolor[i] = POLYRCOLOR[r];
		polygons->gcolor[i] = POLYGCOLOR[r];
		polygons->bcolor[i] = POLYBCOLOR[r];
		polygons->acolor[i] = POLYALPHA[r]; // transparency
	}
	else if(r<nsurf)
	{
		count2+=bound[r];
		++r;
		      
	}
}


  for(j=0;j<nconf;j++){
    fgets(line3,MAXLINE,fp); /* skip over header */
    fgets(line3,MAXLINE,fp2); /* skip over header */
    for(i=0;i<npolygons;i++){
      fscanf(fp,"%g %g %g %g %g %g %g %g %g",
         &(polygons->x1[j][i]),&(polygons->y1[j][i]),&(polygons->z1[j][i]),
         &(polygons->x2[j][i]),&(polygons->y2[j][i]),&(polygons->z2[j][i]),
         &(polygons->x3[j][i]),&(polygons->y3[j][i]),&(polygons->z3[j][i]));
      fscanf(fp2,"%g %g %g %g %g %g %g %g %g",
         &(polygons->nx1[j][i]),&(polygons->ny1[j][i]),&(polygons->nz1[j][i]),
         &(polygons->nx2[j][i]),&(polygons->ny2[j][i]),&(polygons->nz2[j][i]),
         &(polygons->nx3[j][i]),&(polygons->ny3[j][i]),&(polygons->nz3[j][i]));
    }
    fgets(line3,MAXLINE,fp); /* read to end of line */
  }
  fclose(fp);

/*#ifdef DEBUG
  for(j=0;j<nconf;j++){
    for(i=0;i<npolygons;i++){
      printf("%d %d %9g %9g %9g %9g %9g %9g %9g %9g %9g\n",j,i,
         polygons->x1[j][i],polygons->y1[j][i],polygons->z1[j][i],
         polygons->x2[j][i],polygons->y2[j][i],polygons->z2[j][i],
         polygons->x3[j][i],polygons->y3[j][i],polygons->z3[j][i]);
    }
  }
#endif

*/
  *npoly = npolygons;
}


/**********************************************************************/
void read_ribbons(char *filename,int *nribb,RIBBONS *ribbons)
{
  int i,j,nribbons,nconf;
  char line[MAXLINE],name[MAXLINE];
  FILE *fp;
  
  /* open ribbon file */
  if ( (fp = fopen(filename,"r")) == NULL){
    sprintf(line,"can't open ribbon filename \"%s\"",filename);
    md_warning(line);
    return;
  }
  
  /* read header */
  if(fgets(line,MAXLINE,fp)==NULL){
    sprintf(line,"can't get header of ribbon file %s",filename);
    md_warning(line);
    return;
  }

  if((sscanf(line,"%s %d %s",name,&nribbons,line)!=3) && 
     (strcmp(name,"#")!=0)){
    fprintf(stderr,"ERROR: something is wrong with the header of file %s\n",
	    filename);
    return;
  }

  printf("nribbons = %d (remark: %s)\n",nribbons,line);
  /* count the configurations */
  nconf = 0;
  while(fgets(line,MAXLINE,fp) != NULL){
    nconf++;
    for(i=1;i<nribbons;i++){
      fgets(line,MAXLINE,fp);
    }
    fgets(line,MAXLINE,fp);
  }
  /* reset the file pointer */
  rewind(fp);

  if(nconf==0){
    fprintf(stderr,"ERROR: Didn't find any configurations?!!!\n");
    exit(1);
  }
  fprintf(stdout,"Number of Ribbon configs = %d\n",nconf);
  ribbons->nconf = nconf;

  /* allocate memeory */
  ribbons->x1 = fmatrix(0,nconf-1,0,nribbons-1);
  ribbons->y1 = fmatrix(0,nconf-1,0,nribbons-1);
  ribbons->z1 = fmatrix(0,nconf-1,0,nribbons-1);

  ribbons->rcolor = (float *)malloc(nribbons*sizeof(float));
  ribbons->gcolor = (float *)malloc(nribbons*sizeof(float));
  ribbons->bcolor = (float *)malloc(nribbons*sizeof(float));

  for(i=0;i<nribbons;i++){ 
    ribbons->rcolor[i] = 0.3;
    ribbons->gcolor[i] = 0.3;
    ribbons->bcolor[i] = 0.3;
  }

  for(j=0;j<nconf;j++){
    fgets(line,MAXLINE,fp); /* skip over header */
    for(i=0;i<nribbons;i++){
      fscanf(fp,"%g %g %g",
         &(ribbons->x1[j][i]),&(ribbons->y1[j][i]),
         &(ribbons->z1[j][i]));
      
    }
    fgets(line,MAXLINE,fp); /* read to end of line */
  }
  fclose(fp);

#ifdef DEBUG
  for(j=0;j<nconf;j++){
    for(i=0;i<nribbons;i++){
      printf("%d %d %9g %9g %9g\n",j,i,
         ribbons->x1[j][i],ribbons->y1[j][i],ribbons->z1[j][i]);
    }
  }
#endif

  *nribb = nribbons;
}
  


/* #define PRINT_ATOMS */



