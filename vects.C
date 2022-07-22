/*!
  \file = vects.C
  file holds the class fuctions for displaying vectors
*/

#include <GL/gl.h>
#include <math.h>
#include "glsetvalues.h++"
#include "typedefs.h"
#include "vectors.h++"

// initialization of static variables

float VECTS::linewidth=2;
float VECTS::scale=1;

void VECT::set(float const vec[2*DIM])
{
  int i;
  for(i=0;i<2*DIM;i++) _x[i] = vec[i];
  for(i=0;i<DIM;i++) _color[i] = defveccolors[i];
}

void VECT::print(FILE *fp)
{
  for(int i=0;i<DIM;i++){
    fprintf(fp,"dim = %d x=%g dx=%g color=%g\t",i,_x[i],_x[i+DIM],_color[i]);
  }
}


VECTS::VECTS(void){ // constuctor 
  _nvects = -1;
  _vect = (VECT *)NULL;
}

VECTS::~VECTS(void){
  _nvects=-1;
  if(_vect != (VECT *)NULL) free(_vect);
}

void VECTS::set(int nvectors)
{
  _nvects = nvectors;
  if(_vect != (VECT *)NULL) free(_vect);
  _vect = (VECT *)malloc(nvectors*sizeof(VECT));
}

static void drawVector(int solid,int quality,float radius,float length){
  GLUquadricObj *quadObj;
  glPushMatrix();
  glRotatef(90.0, 1.0, 0.0, 0.0);

  /* Draw stem */
  quadObj = gluNewQuadric ();
  if(solid==1) gluQuadricDrawStyle(quadObj,(GLenum)GLU_FILL);
  else gluQuadricDrawStyle(quadObj,(GLenum)GLU_LINE);
  gluQuadricNormals(quadObj,(GLenum)GLU_SMOOTH);
#ifdef CYLINDER_VEC
  gluCylinder (quadObj, 2.6, 2.6,length,quality, 1);   
  gluDeleteQuadric(quadObj);
#else      
  gluCylinder (quadObj, radius, radius, 3.*length/5., quality, 1);
  gluDeleteQuadric(quadObj);

  /* Draw Cone */
  glTranslatef(0.,0,3.*length/5.);
  quadObj = gluNewQuadric ();
  if(solid==1) gluQuadricDrawStyle(quadObj,(GLenum)GLU_FILL);
  else gluQuadricDrawStyle(quadObj,(GLenum)GLU_LINE);
  gluQuadricNormals (quadObj,(GLenum)GLU_SMOOTH);
  gluCylinder (quadObj, 2.*radius, 0, 2.*length/5.,quality, 1);  
  gluDeleteQuadric(quadObj);

  /* Draw Cap on Cone */
  quadObj = gluNewQuadric ();  
  if(solid==1) gluQuadricDrawStyle(quadObj,(GLenum)GLU_FILL);
  else gluQuadricDrawStyle (quadObj,(GLenum)GLU_LINE);
  gluDisk(quadObj,0.,2.*radius,quality,2);
  gluDeleteQuadric(quadObj);
#endif
  glPopMatrix ();
}

void VECT::display(SETVALUES &sv){
  float dx[DIM];
  glColor3fv(_color);
  if ( !sv.vector_cyl ){
    int k;
    glBegin(GL_LINES);
    for(k=0;k<DIM;k++) dx[k] = _x[k]-sv.xoff[k];
    glVertex3fv(dx);   
    for(k=0;k<DIM;k++) dx[k] += _x[k+DIM]*VECTS::scale;      
    glVertex3fv(dx);
    glEnd();
  } else {
    int k;
    glPushMatrix();
    for(k=0;k<DIM;k++) dx[k] = _x[k]-sv.xoff[k];
    glTranslatef(dx[0],dx[1],dx[2]);
    for(k=0;k<DIM;k++) dx[k] = _x[k+DIM]*VECTS::scale;
    float r=0;
    for(k=0;k<DIM;k++) r += dx[k]*dx[k]; 
    r = sqrt(r);
    float a = acos(dx[1]/r)*180./M_PI;
    glRotatef(-a+180,-dx[2]/r,0.0,dx[0]/r);
    drawVector(sv.solid,sv.cylinder_quality,sv.bond_radius,r);
    glPopMatrix ();
  }
}

void displayVectors(VECTS &vects,SETVALUES &sv)
{
#ifdef DEBUG
  printf("displaying vectors %d\n",!sv.vector_cyl);
#endif
  VECT *vectors = vects.vect();

  glLineWidth(VECTS::linewidth);
  
  if(sv.solid) glShadeModel(GL_SMOOTH);
  else glShadeModel(GL_FLAT);

  glPushMatrix();
  for(int i=0;i<vects.nvects();i++){
    vectors[i].display(sv);
  }
  glPopMatrix();
}


/*!
  read in vectors from a that is associated with an atoms 
*/
int readVectorsAtoms(char *filename,int *ncvec,VECTS **vectors,
		     int natoms,float **x)
{
  int i,j,nvectors,nconf;
  char line[MAXLINE],name[MAXLINE];
  FILE *fp;
  
  /* open vector file */
  if ( (fp = fopen(filename,"r")) == NULL){
    sprintf(line,"can't open vector filename \"%s\"",filename);
    md_warning(line);
    return 1;
  }
  
  /* read header */
  if(fgets(line,MAXLINE,fp)==NULL){
    sprintf(line,"can't get header of vector file %s",filename);
    md_warning(line);
    return 1;
  }

  if((sscanf(line,"%s %d",name,&nvectors)!=2) && (strcmp(name,"#")!=0)){
    fprintf(stderr,"ERROR: something is wrong with the header of file %s\n",
	    filename);
    return 1;
  }

  if(nvectors != natoms){
    fprintf(stderr,"ERROR: nvects %d != %d natoms in \"%s\"\n",
	    nvectors,natoms,filename);
    return 1;
  }

  printf("nvectors = %d, line = %s",nvectors,line);
  /* count the configurations */
  nconf = 0;
  while(fgets(line,MAXLINE,fp) != NULL){
    nconf++;
    for(i=1;i<nvectors;i++){
      fgets(line,MAXLINE,fp);
    }
    fgets(line,MAXLINE,fp);
  }
  /* reset the file pointer */
  rewind(fp);

  if(nconf==0){
    fprintf(stderr,"ERROR: Didn't find any configurations?!!!\n");
    *ncvec = 0;
    return 1;
  }
  fprintf(stdout,"Number of Vector configs = %d\n",nconf);

  /* allocate memeory testing if we have already done to avoid mem leak */
  if(*vectors == NULL) free(*vectors);
  *vectors = (VECTS *)malloc(nconf*sizeof(VECTS));
  for(i=0;i<nconf;i++) (*vectors)[i] = VECTS(); // call constructor

  for(j=0;j<nconf;j++){
    fgets(line,MAXLINE,fp); /* read nvect */
    if((sscanf(line,"%s %d",name,&nvectors)!=2) && (strcmp(name,"#")!=0)){
      fprintf(stderr,"ERROR: something is wrong with the header %s\n",
	      filename);
      fprintf(stderr,"\t old file type? make sure that a line \"# num ...\"");
      fprintf(stderr,"seperates each vector config\n");
      return 1;
    }
    if(nvectors != natoms){
      fprintf(stderr,"ERROR: nvects %d != %d natoms in \"%s\" config %d\n",
	      nvectors,natoms,filename,j);
      return 1;
    }

    (*vectors)[j].set(nvectors); //set number of vectors
    VECT *vect = (*vectors)[j].vect();
    for(i=0;i<nvectors;i++){
      fgets(line,MAXLINE,fp); /* read in line */
      float vec[2*DIM];
      if(sscanf(line,"%g %g %g",&(vec[3]),&(vec[4]),&(vec[5])) != 3){
	fprintf(stderr,"ERROR: while reading in vectors! (vec %d:%d)\n",j,i);
	exit(1);
      }
      for(int k=0;k<DIM;k++) vec[k] = x[j][i*DIM+k];
      vect[i].set(vec);
    }
  }
  fclose(fp);

#ifdef DEBUG
  for(j=0;j<nconf;j++){
    VECT *vect = (*vectors)[i].vect();
    for(i=0;i<nvectors;i++){
      fprintf(stdout,"%d ",i);
      vect[i].print(stdout);
      fprintf(stdout,"\n");
    }
  }
#endif

  *ncvec = nconf;
  return 0;
}

/*!
  read in vectors from a file
*/
int readVectors(char *filename,int *ncvec,VECTS **vectors)
{
  int i,j,nvectors,nconf;
  char line[MAXLINE],name[MAXLINE];
  FILE *fp;
  
  /* open vector file */
  if ( (fp = fopen(filename,"r")) == NULL){
    sprintf(line,"can't open vector filename \"%s\"",filename);
    md_warning(line);
    return 1;
  }
  
  /* read header */
  if(fgets(line,MAXLINE,fp)==NULL){
    sprintf(line,"can't get header of vector file %s",filename);
    md_warning(line);
    return 1;
  }

  if((sscanf(line,"%s %d",name,&nvectors)!=2) && (strcmp(name,"#")!=0)){
    fprintf(stderr,"ERROR: something is wrong with the header of file %s\n",
	    filename);
    return 1;
  }

  printf("nvectors = %d, line = %s",nvectors,line);
  /* count the configurations */
  nconf = 0;
  while(fgets(line,MAXLINE,fp) != NULL){
    nconf++;
    for(i=1;i<nvectors;i++){
      fgets(line,MAXLINE,fp);
    }
    fgets(line,MAXLINE,fp);
  }
  /* reset the file pointer */
  rewind(fp);

  if(nconf==0){
    fprintf(stderr,"ERROR: Didn't find any configurations?!!!\n");
    *ncvec = 0;
    return 1;
  }
  fprintf(stdout,"Number of Vector configs = %d\n",nconf);

  /* allocate memeory could use new instead of malloc and construct */
  if(*vectors != NULL) free(*vectors);
  *vectors = (VECTS *)malloc(nconf*sizeof(VECTS));
  for(i=0;i<nconf;i++) (*vectors)[i] = VECTS(); // call constructor

  for(j=0;j<nconf;j++){
    fgets(line,MAXLINE,fp); /* skip over header */
    if((sscanf(line,"%s %d",name,&nvectors)!=2) && (strcmp(name,"#")!=0)){
      fprintf(stderr,"ERROR: something is wrong with the header %s\n",
	      filename);
      fprintf(stderr,"\t old file type? make sure that a line \"# num ...\"");
      fprintf(stderr,"seperates each vector config\n");
      return 1;
    }
    (*vectors)[j].set(nvectors); //set number of vectors
    VECT *vect = (*vectors)[j].vect();
    for(i=0;i<nvectors;i++){
      fgets(line,MAXLINE,fp); /* read in line */
      float vec[2*DIM];
      if(sscanf(line,"%g %g %g %g %g %g",&(vec[0]),&(vec[1]),&(vec[2]),
		&(vec[3]),&(vec[4]),&(vec[5])) != 6){
	fprintf(stderr,"ERROR: while reading in vectors! (vec %d:%d)\n",j,i);
	exit(1);
      }
      vect[i].set(vec);
    }
    // fgets(line,MAXLINE,fp); /* read to end of line */
  }
  fclose(fp);

#ifdef DEBUG
  for(j=0;j<nconf;j++){
    VECT *vect = (*vectors)[i].vect();
    for(i=0;i<nvectors;i++){
      fprintf(stdout,"%d ",i);
      vect[i].print(stdout);
      fprintf(stdout,"\n");
    }
  }
#endif

  *ncvec = nconf;
  return 0;
}

