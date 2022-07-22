
/* 
   Copyright Preston Moore and University of Pennsylvania
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
// #define PRINTATOMS

#define MIN(A,B) (((A)<(B))?(A):(B))
#define MAX(A,B) (((A)>(B))?(A):(B))

// #define OFFSET /* if we want to move to center of box */

/***********************************************************************/
void system_mem(SYSTEM &system){system.atoms = new ATOMS[system.natoms];}
/***********************************************************************/
/*------------------------------------------------------------------*/
/* subroutine to read in configuration file */

void readXYZConfigs(char *filename,SYSTEM &system)
{
  int i,j,natoms,nconf;
  char line[MAXLINE];
  WORD key,name;
  float xmax,xmin,ymax,ymin,zmax,zmin;
  FILE *fp;

  if((fp = fopen(filename,"r")) == NULL){
    fprintf(stderr,"ERROR: can't open %s\n",filename);
    exit(1);
  }

  // read first line to get # of atoms 
  if(fgets(line,MAXLINE,fp) == NULL){
    fprintf(stderr,"ERROR: nothing in the file \"%s\"?\n",filename);
    exit(1);
  }
  sscanf(line,"%d",&natoms);

  if(fgets(line,MAXLINE,fp) == NULL){
    fprintf(stderr,"ERROR: nothing after the atoms \"%s\"?\n",filename);
    exit(1);
  }

  fprintf(stdout,"natoms = %d: %sCounting configurations:\n",natoms,line);
  /* count the configurations and atoms */
  i = 2; /* start from line 2 */
  while(fgets(line,MAXLINE,fp) != NULL){sscanf(line,"%s",key);i++;}
  if(((i-2)%(natoms+2)) != 0){
    fprintf(stderr,"WARNING: # of atoms is not divisible # of lines\n");
    fprintf(stderr,"%d %d %d\n",i,(natoms+2),i%(natoms+2));
  }
  system.natoms = natoms;
  nconf = i/(natoms+2);  
  system.nconf = nconf/(system.iskip+1);
  printf("Number of configurations = %d, Number of Atoms = %d\n",nconf,natoms);
  printf("Number of configurations being stored = %d\n",system.nconf);
  if(nconf==0 || natoms==0) {
    fprintf(stderr,"NO ATOMS?(%d) or CONFIGURATIONS (%d) ",natoms,nconf);
    fprintf(stderr,"-- is file terminated correctly?\n");
    exit(1);
  }
  rewind(fp);   /* rewind and loop over header */  
  system_mem(system);   /* allocate memory */
  system.coords = (COORDS *)malloc(system.nconf*sizeof(COORDS));
  system.cell = (CELLS *)malloc(system.nconf*sizeof(CELLS));

  /* allocate and zero display flag */
  system.idisp = (int *)malloc(system.nconf*sizeof(int));
  for(i=0;i<system.nconf;i++) system.idisp[i] = 0;
  xmin = xmax = ymin = ymax = zmin = zmax = 0.;

  float *x=NULL,*f=NULL;
  int iconf = 0;
  for(j=0;j<nconf;j++){
    fgets(line,MAXLINE,fp);
    sscanf(line,"%d",&i);
    if(i != natoms){
      fprintf(stderr,"ERROR: Number of atoms have changed!\n");
      exit(1);
    }
    fgets(line,MAXLINE,fp);
    if(j%(system.iskip+1)==0){
      /* printf("allocationg memory of config %d\n",iconf); */
      x = system.coords[iconf].setncoords(system.natoms);
      f = system.coords[iconf].setnfact(system.natoms);
    }
    for(i=0;i<natoms;i++){
      fgets(line,MAXLINE,fp);
      if(j%(system.iskip+1)==0){
	sscanf(line,"%s%g%g%g",name,&(x[i*DIM]),&(x[i*DIM+1]),&(x[i*DIM+2]));
	f[i] = 1;
      }
      if(i==0){
	xmin = xmax = x[i*DIM];
	ymin = ymax = x[i*DIM+1];
	zmin = zmax = x[i*DIM+2];
      }
      if(j==0) system.atoms[i].atomName(name);
      xmin = MIN(xmin,x[i*DIM]);      xmax = MAX(xmax,x[i*DIM]);
      ymin = MIN(ymin,x[i*DIM+1]);    ymax = MAX(ymax,x[i*DIM+1]);
      zmin = MIN(zmin,x[i*DIM+2]);    zmax = MAX(zmax,x[i*DIM+2]);
      
#ifdef PRINTATOMS
      // #ifdef DEBUG
      printf("atoms: %d %d %g %g %g\n",j,i,x[i*DIM],x[i*DIM+1],x[i*DIM+2]);
      system.atoms[i].print(stdout);
      printf("\n");
#endif
    }
    switch(j%4){
    case 0:printf("[|]"); break;
    case 1:printf("[/]"); break;
    case 2:printf("[-]"); break;
    case 3:printf("[\\]"); break;
    }
    if(j%10==0)printf("  %d ", j);
    printf("\r");
    fflush(stdout);
    
//     fprintf(stdout,"."); fflush(stdout); 
//     if(j%50==0) printf("\r%80s\r%d "," ",j); 
//     else if(j%10==0) printf("%d",j); 
    for(i=0;i<9;i++) system.cell[iconf][i] = 0.;
    system.cell[iconf][0] = xmax-xmin;
    system.cell[iconf][4] = ymax-ymin;
    system.cell[iconf][8] = zmax-zmin;
    if(j%(system.iskip+1)==0) iconf++;
  }

  fclose(fp);

  /* move atoms offset */
#ifdef OFFSET
  for(i=0;i<system.nconf;i++){
    float xoff[3];
    for(int k=0;k<DIM;k++) xoff[i] = 0;
    x = system.coords[i].getx();
    for(int j=0;j<system.natoms;j++){
      for(int k=0;k<DIM;k++) xoff[i] += x[i*DIM+k];
    }
    for(int k=0;k<DIM;k++) xoff[i] /= float(natoms);
    for(int j=0;j<system.natoms;j++){
      for(int k=0;k<DIM;k++) x[i] -= xoff[i];
    }
  }
#endif
}

static int isatom(char *key)
{
  if(strncasecmp(key,"ATOM",4)==0) return 1;
  if(strncasecmp(key,"HETATM",6)==0) return 1;
  return 0;
}
/*------------------------------------------------------------------*/
/* subroutine to read in configuration file */

void readPdbConfigs(char *filename,SYSTEM &system)
{
  int i,natoms,nconf,icell;
  char line[MAXLINE];
  WORD key,name;
  float xmax,xmin,ymax,ymin,zmax,zmin;
  FILE *fp;

  if((fp = fopen(filename,"r")) == NULL){
    fprintf(stderr,"ERROR: can't open %s\n",filename);
    return;
  }

  /* count the configurations and atoms */
  i = nconf = natoms = 0;
  while(fgets(line,MAXLINE,fp) != NULL){
    sscanf(line,"%s",key);
    if((strcasecmp(key,"TER")==0 || strcasecmp(key,"END")==0) && i==1) {
      nconf++;
      i = 0;
    }
    if(nconf==0 && isatom(key)){
      natoms++;
    }
    if(isatom(key)) i = 1;
  }

  system.nconf = nconf;
  system.natoms = natoms;
  printf("Number of configurations = %d, Number of Atoms = %d\n",nconf,natoms);
  if(nconf==0 || natoms==0) {
    fprintf(stderr,"NO ATOMS? (%d) or CONFIGURATIONS (%d)",natoms,nconf);
    fprintf(stderr,"-- is file terminated correctly?\n");
    exit(1);
  }
  
  /* rewind and loop over header */
  rewind(fp);

  /* allocate memory */
  system_mem(system);
  
  system.coords = (COORDS *)malloc(nconf*sizeof(COORDS));
  system.cell = (CELLS *)malloc(system.nconf*sizeof(CELLS));

  /* allocate and zero display flag */
  system.idisp = (int *)malloc(nconf*sizeof(int));
  for(i=0;i<nconf;i++) system.idisp[i] = 0;

  float *x=NULL,*f=NULL;

  natoms = nconf = 0;
  xmin = xmax = ymin = ymax = zmin = zmax = 0.;
  while(fgets(line,MAXLINE,fp) != NULL){
    sscanf(line,"%s",key);
    if((strcasecmp(key,"TER")==0 || strcasecmp(key,"END")==0) && natoms!=0){

    switch(nconf%4){
    case 0:printf("[|]"); break;
    case 1:printf("[/]"); break;
    case 2:printf("[-]"); break;
    case 3:printf("[\\]"); break;
    }
    if(nconf%10==0)printf("  %d ", nconf);
    printf("\r");
    fflush(stdout);

//       fprintf(stdout,"."); fflush(stdout); 
//       if(nconf%50==0) printf("\r%80s\r%d "," ",nconf); 
//       else if(nconf%10==0) printf("%d",nconf); 

      for(icell=0;icell<9;icell++) system.cell[nconf][icell] = 0.;
      system.cell[nconf][0] = xmax-xmin;
      system.cell[nconf][4] = ymax-ymin;
      system.cell[nconf][8] = zmax-zmin;
      nconf++;
      if(natoms != system.natoms){
        fprintf(stderr,"Number of atoms differ in file ... exiting\n");
        exit(1);
      }
      natoms=0;
      xmin = xmax = ymin = ymax = zmin = zmax = 0.;
    }
    if(isatom(key)){
      if(nconf==0){
	int igroup;
        sscanf(line,"%*s %*d %s",name);
	system.atoms[natoms].atomName(name);
        sscanf(line,"%*s %*d %*s %s %d",name,&igroup);
	system.atoms[natoms].igroup(igroup);
	system.atoms[natoms].groupName(name);
      }
      if(natoms==0) {
	x = system.coords[nconf].setncoords(system.natoms);
	f = system.coords[nconf].setnfact(system.natoms);
      }
      /* sscanf(line,"%*s %*d %*s %*s %*d %g %g %g", */
      sscanf(line,"%*30c%g %g %g",
	     &(x[natoms*DIM]),&(x[natoms*DIM+1]),&(x[natoms*DIM+2]));
      f[natoms] = 1;

      xmin = MIN(xmin,x[natoms*DIM]);
      xmax = MAX(xmax,x[natoms*DIM]);
      ymin = MIN(ymin,x[natoms*DIM+1]);
      ymax = MAX(ymax,x[natoms*DIM+1]);
      zmin = MIN(zmin,x[natoms*DIM+2]);
      zmax = MAX(zmax,x[natoms*DIM+2]);

#ifdef PRINTATOMS
      // #ifdef DEBUG
      printf("atoms: %d %d %s %s %g %g %g\n",nconf,natoms,
	     system.atoms[natoms].atomName(),system.atoms[natoms].groupName(),
	     x[natoms*DIM],x[natoms*DIM+1],x[natoms*DIM+2]);
#endif
      natoms++;
      if(natoms > system.natoms){
        fprintf(stderr,"ERROR: Number of atoms have changed!\n");
        exit(1);
      }
    }
  }
  fclose(fp);

  /* move atoms offset */
#ifdef OFFSET
  float xoff[3];
  xoff[0] = system.cell[0][0]/2.;
  xoff[1] = system.cell[0][4]/2.;
  xoff[2] = system.cell[0][8]/2.;
  for(i=0;i<nconf;i++){
    x = system.coords[i].getx();
    for(int j=0;j<system.natoms;j++){
      for(int k=0;k<DIM;k++) x[i] -= xoff[i];
    }
  }
#endif
}
/*------------------------------------------------------------------*/
void read_jsmovie(char *filename,SYSTEM &system)
{
  int i,j;
  WORD line;
  FILE *fp;
  float *x,*f;

  if((fp = fopen(filename,"r")) == NULL){
    fprintf(stderr,"ERROR: can't open %s\n",filename);
    exit(1);
  }
  printf("Reading in John Shelly movie format\n");

  fgets(line,MAXLINE,fp);
  sscanf(line,"%d",&(system.nconf));
  fgets(line,MAXLINE,fp);
  system.jscell = (JSCELLS *)malloc(sizeof(JSCELLS));
  fgets(line,MAXLINE,fp);
  printf("%s",line);
  sscanf(line,"%d %d",&(system.jscell->ncorners),&(system.jscell->nlines));
  system.jscell->x=(float *)malloc(3*system.jscell->ncorners*sizeof(float));
  system.jscell->corn1 = (int *)malloc(system.jscell->nlines*sizeof(int));
  system.jscell->corn2 = (int *)malloc(system.jscell->nlines*sizeof(int));
  for(i=0;i<system.jscell->nlines;i++){
    fgets(line,MAXLINE,fp);
    sscanf(line,"%d %d",
	   &(system.jscell->corn1[i]),&(system.jscell->corn2[i]));
  }
  fgets(line,MAXLINE,fp);

  sscanf(line,"%d",&system.natoms);
  fgets(line,MAXLINE,fp);
  fgets(line,MAXLINE,fp);
  
  printf("nconfigs = %d, ntoms = %d\n",system.nconf,system.natoms);

  /* allocate memory */
  system.idisp = (int *)malloc(system.nconf*sizeof(int));
  system_mem(system);
  system.coords = (COORDS *)malloc(system.nconf*sizeof(COORDS));
  for(i=0;i<system.nconf;i++) system.idisp[i] = 0;

  for (j=0;j<system.nconf;j++){
    for(i=0;i<system.jscell->ncorners;i++){
      fgets(line,MAXLINE,fp);
      sscanf(line,"%g %g %g",&(system.jscell->x[i*3]),
	     &(system.jscell->x[i*3+1]),&(system.jscell->x[i*3+2]));
    }
    x = system.coords[j].setncoords(system.natoms);
    f = system.coords[j].setnfact(system.natoms);
    for(i=0;i<system.natoms;i++){
      if(fgets(line,MAXLINE,fp) == NULL){
        fprintf(stderr,"ERROR: something is wrong with configuration file\n");
        exit(1);
      }
      int itype;
      if(sscanf(line,"%g %g %g %d",&(x[i*DIM]),&(x[i*DIM+1]),&(x[i*DIM+2]),
		&itype) != 4){
	fprintf(stderr,"ERROR: something is wrong with configuration file\n");
        exit(1);
      }
      system.atoms[i].itype(itype);
      f[i] = 1;
    }
  }
  fclose(fp);
}
/*******************************************************/
/* subroutine to read in configuration file */

void read_configs(char *filename,SYSTEM &system)
{
  int *sconf,natoms,i,natoms_now,nconf;
  float dt;
  WORD line;
  FILE *fp;

  sconf = &(system.nconf);
  natoms = system.natoms;

  if((fp = fopen(filename,"r")) == NULL){
    fprintf(stderr,"ERROR: can't open %s\n",filename);
    exit(1);
  }

  /* read in header info */
  
  fgets(line,MAXLINE,fp);
  if(line[0] == '#'){
    if(sscanf(line,"%*s %d %d %g",&natoms_now,&nconf,&dt) != 3){
      fprintf(stderr,"ERROR: something is wrong with config header\n");
      exit(1);
    }
  } else {
    printf("Header is not a config file ... assuming init file\n");
    if(sscanf(line,"%d",&natoms_now) != 1){
      fprintf(stderr,"ERROR: something is wrong with config header\n");
      exit(1);
    }
    nconf = -1;
    dt = 1.;
  }

  if(natoms != natoms_now){
    fprintf(stderr,"ERROR: number of atoms do not match %d vs %d\n",
	    natoms,natoms_now);
    exit(1);
  }
    
  fprintf(stdout,"# of atoms = %d, number of time steps per # of conf = %d\n",
	  natoms,nconf);
  fprintf(stdout,"dt = %g ps/time step\nCounting configurations\n",dt);

  /* count the configurations */
  if(nconf!=-1){
    nconf = 0;
    while(fgets(line,MAXLINE,fp) != NULL){
      nconf++;
      for(i=1;i<natoms;i++){
        fgets(line,MAXLINE,fp);
      }
      fgets(line,MAXLINE,fp);

    switch(nconf%4){
    case 0:printf("[|]"); break;
    case 1:printf("[/]"); break;
    case 2:printf("[-]"); break;
    case 3:printf("[\\]"); break;
    }
    if(nconf%10==0)printf("  %d ", nconf);
    printf("\r");
    fflush(stdout);

//       fprintf(stdout,"."); fflush(stdout); 
//       if(nconf%50==0) printf("\r%80s\r%d "," ",nconf); 
//       else if(nconf%10==0) printf("%d",nconf); 
    }
  } else {
    nconf = 1;
  }
  printf("\n");
  if(nconf==0){
    fprintf(stderr,"ERROR: Didn't find any configurations?!!!\n");
    exit(1);
  }

  *sconf = nconf;
  printf("\nNumber of configurations = %d\nAllocating memory\n",nconf);

  /* allocate and zero display flag */
  system.idisp = (int *)malloc(nconf*sizeof(int));
  for(i=0;i<nconf;i++) system.idisp[i] = 0;

  /* allocate memory */
  system.coords = (COORDS *)malloc(nconf*sizeof(COORDS));
  system.cell = (CELLS *)malloc(nconf*sizeof(CELLS));

  printf("Assigning atoms positions\n");
  /* rewind and loop over header */
  rewind(fp);
  fgets(line,MAXLINE,fp);

  nconf = 0;
  while(fgets(line,MAXLINE,fp) != NULL && (nconf< *sconf)){
    int nread;
    float* x = system.coords[nconf].setncoords(natoms);
    float* f = system.coords[nconf].setnfact(natoms);
    nread = sscanf(line,"%g %g %g %g",&(x[0]),&(x[1]),&(x[2]),&(f[0]));
    if(nread ==3){
      f[0] = 1;
    } else if (nread !=4){
      fprintf(stderr,"ERROR: something is wrong with configuration file\n");
      fprintf(stderr,"ERROR: %d (1)\n",nconf);
      fprintf(stderr,"%s\n",line);
      fprintf(stderr,"Bailing out!\n");
      break;
      // exit(1);
    }
    for(i=1;i<natoms;i++){
      if(fgets(line,MAXLINE,fp) == NULL){
        fprintf(stderr,"ERROR: something is wrong with configuration file\n");
        fprintf(stderr,"ERROR: %d (2)\n",nconf);
	fprintf(stderr,"%s\n",line);
	fprintf(stderr,"Bailing out!\n");
	break;
        // exit(1);
      }
      int j=i*3;
      nread = sscanf(line,"%g %g %g %g",&(x[j]),&(x[j+1]),&(x[j+2]),&(f[i]));
      if(nread ==3){
	f[i] = 1;
      } else if (nread !=4){
        fprintf(stderr,"ERROR: something is wrong with configuration file\n");
        fprintf(stderr,"ERROR: %d (3)\n",nconf);
        fprintf(stderr,"%s\n",line);
	fprintf(stderr,"Bailing out!\n");
	break;
        // exit(1);
      }
    }
    if(i!=natoms) break;
    for(i=0;i<9;i++){
      if(fscanf(fp,"%g",&system.cell[nconf][i]) != 1){
        fprintf(stderr,"ERROR: something is wrong with configuration file\n");
        fprintf(stderr,"\twhile reading in the cell at config #%d\n",nconf);
        exit(1);
      }
    }
    /* read the end of line */
    if(fgets(line,MAXLINE,fp) == NULL){
      fprintf(stderr,"ERROR: something is with the configuration file\n");
      fprintf(stderr,"ERROR: while readin the cell at config %d (3)\n",nconf);
      exit(1);
    }
    
    nconf++;

    switch(nconf%4){
    case 0:printf("[|]"); break;
    case 1:printf("[/]"); break;
    case 2:printf("[-]"); break;
    case 3:printf("[\\]"); break;
    }
    if(nconf%10==0)printf(" %d ", nconf);
    printf("\r");
    fflush(stdout);
//     fprintf(stdout,"."); fflush(stdout); 
//     if(nconf%50==0) printf("\r%80s\r%d "," ",nconf); 
//     else if(nconf%10==0) printf("%d",nconf); 
  }
  fclose(fp);
  fprintf(stdout,"\n");
  if(nconf!= *sconf) {
    printf("You are on your own\n!");
    *sconf = nconf;
  }

#ifdef DEBUG
  for(nconf=0;nconf<*sconf;nconf++){
    printf("config = %d\n",nconf);
    float* x = system.coords[nconf].getx();
    float* f = system.coords[nconf].getf();
    for(i=0;i<natoms;i++){
      printf("pos= %g %g %g f=%g\n",x[i*3],x[i*3+1],x[i*3+2],f[i]); 
    }
  }
#endif
}
/*------------------------------------------------------------------------*/
void read_top_file(SYSTEM &system,int nspec,SPECIES *spec_now,
		   ATOM_TOPOL **atom_topol,int **ibo,int **jbo, WORD **tbo)
{
  int i,j,ispec,iatom,ioff,ib_now,ib_off,ib;
  SPECIES *spec_root;
  FILE *fp;
  
  spec_root = spec_now;
  system.natoms = 0;
  ib_off = 0;

  for(ispec=0;ispec<nspec;ispec++,spec_now=spec_now->next){
    
    /* open parameter file */
    fp = fopen(spec_now->filename,"r");
    if (fp == NULL){
      fprintf(stderr,"ERROR: can't open set file (%s)\n",spec_now->filename);
      exit(1);
    } else {
      fprintf(stdout,"Reading in parameter from file %s\n",
	      spec_now->filename);
    }
    get_species(fp,spec_now);
    if(spec_now->napm <= 0)
      fprintf(stderr,"ERROR, %s has %d atoms!\n",
	      spec_now->filename,spec_now->napm);

    atom_topol[ispec] = (ATOM_TOPOL *)malloc(spec_now->napm*
					     sizeof(atom_topol[0][0]));
    get_atom(fp,spec_now->filename,spec_now->napm,atom_topol[ispec]);

    /* bonding */
    // if(spec_now->nbond > 0){
      ibo[ispec] = (int *)malloc(spec_now->nbond*sizeof(int));
      jbo[ispec] = (int *)malloc(spec_now->nbond*sizeof(int));
      tbo[ispec] = (WORD *)malloc(spec_now->nbond*sizeof(WORD));
      if(ibo[ispec] == NULL || ibo[ispec] == NULL || ibo[ispec] == NULL ){
	fprintf(stderr,"Can't allocate memory\n");
	exit(1);
      }
      get_bond(fp,spec_now->filename,spec_now->nbond,ibo[ispec],jbo[ispec],tbo[ispec]);
      for(i=0;i<spec_now->nbond;i++){
	for(j=0;j<spec_now->napm;j++){
	  if(atom_topol[ispec][j].atm_idx == ibo[ispec][i]) ibo[ispec][i] = j;
	  if(atom_topol[ispec][j].atm_idx == jbo[ispec][i]) jbo[ispec][i] = j;
	}
      }
      // }
    fclose(fp);
    system.natoms += spec_now->napm*spec_now->nmol;
    ib_off += spec_now->nbond*spec_now->nmol;
  }

  system.bonds = (BONDS *)malloc(sizeof(BONDS));
  system.bonds[0].nbonds = ib_off;
  printf("Total number of nbonds = %d\n",system.bonds[0].nbonds);

  system.bonds[0].bond = (BOND *)malloc(ib_off*sizeof(BOND));
  ib_now = ib_off = iatom = 0;
  /* loop over the species and determine bonded types */
  for(ispec=0,spec_now=spec_root;ispec<nspec;
      ispec++,spec_now=spec_now->next){
    
    /* loop over the bonds */
    for(ib=0;ib<spec_now->nbond;ib++){
      /* fill up bond lists */
      for(j=0;j<spec_now->nmol;j++){
        ioff = iatom + j*spec_now->napm;
	int ibi = ioff + ibo[ispec][ib];	
	int ibj = ioff + jbo[ispec][ib];
        system.bonds[0].bond[ib_now].set(ibi,ibj);
        ib_now++;
      }
    }
    iatom += spec_now->napm*spec_now->nmol;
  }

#ifdef DEBUG
  printf("bond connectivity\n");
  for(i=0;i<system.bonds[0].nbonds;i++){
    printf("%d %d %d %d\n",ib_now,i,system.bonds[0].bond[i].ib(),system.bonds[0].bond[i].jb());
  }
#endif
}

/*------------------------------------------------------------------------*/

void read_topfile(SYSTEM &system,int nspec,SPECIES *spec_root)
{
  int i,j,k,ispec,ntypes,ioff;
  int iatom,ngroup,natoms,ngpm;
  int **ibo,**jbo; WORD **tbo;
  SPECIES *spec_now;
  WORD tgroup,*labels_atoms,*labels_group;
  ATOM_TOPOL **atom_topol;
  
  spec_now = spec_root;
  /* allocate temparary memory */
  atom_topol = (ATOM_TOPOL **)malloc(nspec*sizeof(ATOM_TOPOL *));

  /* bonds */
  ibo = (int **)malloc(nspec*sizeof(int*));
  jbo = (int **)malloc(nspec*sizeof(int*));
  tbo = (WORD **)malloc(nspec*sizeof(WORD*));

  /* read in all connectivity data from files */
  read_top_file(system,nspec,spec_now,atom_topol,ibo,jbo,tbo);
  
  printf("Number of total atoms = %d\n",system.natoms);

  natoms = system.natoms;

  /* allocate memeory for maps and charges and masses */
  system_mem(system);

  labels_atoms = (WORD *)malloc(natoms*sizeof(WORD));
  labels_group = (WORD *)malloc(natoms*sizeof(WORD));

  /* get all the atom types */
  ngroup = ntypes = iatom = 0;

  /* fill in the maps atom maps*/
  for(ispec=0,spec_now=spec_root;ispec<nspec;ispec++,spec_now=spec_now->next){
    
    /* set up to check for group type */
    strcpy((labels_group)[ngroup],atom_topol[ispec][0].group);
    strcpy(tgroup,atom_topol[ispec][0].group);
    ngpm = 0;

    /* loop over atoms in the molecule */
    for(j=0;j<spec_now->napm;j++){
      
      /* check for new atom type */
      for(i=0;i<ntypes;i++){
        if(strcasecmp(atom_topol[ispec][j].type,labels_atoms[i]) == 0) {
          break;
        }
      }
      if(i==ntypes){
        strcpy(labels_atoms[ntypes],atom_topol[ispec][j].type);
        ntypes++;
      }
      
      /* check for new group type */
      if(strcasecmp(atom_topol[ispec][j].group,tgroup) != 0) {
        strcpy(tgroup,atom_topol[ispec][j].group);
        ngpm++;
      }
      
      /* assign all different types */
      for(k=0;k<spec_now->nmol;k++){
        ioff = iatom + j + k*spec_now->napm;
        system.atoms[ioff].itype(i);
        system.atoms[ioff].iatom(j);
        system.atoms[ioff].imole(k);
        system.atoms[ioff].ispec(ispec);
        system.atoms[ioff].igroup(ngpm+ngroup+k);
        system.atoms[ioff].amass(atom_topol[ispec][j].mass);
        system.atoms[ioff].qch(atom_topol[ispec][j].charge);
        system.atoms[ioff].atomName(atom_topol[ispec][j].type);
	system.atoms[ioff].moleName(spec_now->name);
        system.atoms[ioff].groupName(atom_topol[ispec][j].group);
      }
    }
    iatom += spec_now->napm*spec_now->nmol;
    ngroup += ngpm*spec_now->nmol;
  }
  
#ifdef DEBUG
  printf("printing atoms = %d\n",natoms);
  for(ioff=0;ioff<natoms;ioff++){
    printf("%d %d %d %d %d %d a=%s m=%s g=%s %g %g\n",ioff,
	   system.atoms[ioff].ispec(),system.atoms[ioff].imole(),
	   system.atoms[ioff].igroup(),system.atoms[ioff].iatom(),
	   system.atoms[ioff].itype(),
	   system.atoms[ioff].atomName(),system.atoms[ioff].moleName(),
	   system.atoms[ioff].groupName(),
	   system.atoms[ioff].amass(),system.atoms[ioff].qch());
  }
  printf("There are %d groups and %d types of atoms\n",ngroup,ntypes);
#endif
  
  /* free temporary storage */
  for(i=0;i<nspec;i++){
    free(atom_topol[i]);
    free(ibo[i]); free(jbo[i]); free(tbo[i]);
  }
  free(atom_topol);
  free(ibo); free(jbo); free(tbo);
  free(labels_atoms); free(labels_group);

}
/***********************************************************************/
/* routine to read in data */
void readInput(SYSTEM &system,SETVALUES &sv)
{
  int i,nspec;
  SPECIES spec_root,*spec_now,*spec_next;

  spec_now = &spec_root;
   
  switch(sv.movie_type){
  case 1:
    read_jsmovie(sv.setfile,system);
    read_attributes(sv.attfile,system.natoms,system.atoms);
    searchBonds(system);
    break;
  case 2:
    readPdbConfigs(sv.setfile,system);
    read_attributes(sv.attfile,system.natoms,system.atoms);
    searchBonds(system);
    break;
  case 3:
    readXYZConfigs(sv.setfile,system);
    read_attributes(sv.attfile,system.natoms,system.atoms);
    searchBonds(system);
    break;
  case 0:
  default :
    /* read in the set file which specifies molecule types */
    if ( sv.setfile == NULL) {
      WORD name;
      printf("Enter set file: ");
      scanf("%s",name);
      sv.setfile = strdup(name);
    }
    read_setfile(sv.setfile,&nspec,spec_now);
    /* read in the parameters */
    read_topfile(system,nspec,spec_now);
    
    /* free spec_root children */
    spec_now = spec_now->next;
    for ( i=0; i<nspec; i++ ) {
      spec_next = spec_now->next; free(spec_now); spec_now = spec_next;
    }
    
    if ( sv.confile == NULL ) {
      WORD name;
      printf("Enter configuration filename :");
      scanf("%s",name);
      sv.confile = strdup(name);
    }
    /* finally read in the configurations */
    read_configs(sv.confile,system);

    // setup all the bonds
    system.bonds = (BONDS *)realloc(system.bonds,system.nconf*sizeof(BONDS));
    for(i=1;i<system.nconf;i++)
      system.bonds[i].set1(system.bonds[0].nbonds,system.bonds[0].bond);
    
    /* read int the attributes of the atoms */
    read_attributes(sv.attfile,system.natoms,system.atoms);
  }  
  
  for(i=0;i<system.natoms;i++) system.atoms[i].ishow(true);
  setValues(sv,system);
}


/* file to read in the set file with specifies the molecules */
/*--------------------------------------------------------------*/
void read_setfile(char *setfile,int *nspec,SPECIES *spec_root)
{
  int i,ich,nline,nch,num_keys,icase,ispec;
  SPECIES *spec_now=spec_root;

  char line[MAXLINE];
  WORD metakey,dict[4];
  int  num_dict=4,num_found[4];
  KEY key_root,*key_now;
  FILE *fp;

  strcpy(dict[0],"mol_parm_file"); strcpy(dict[1],"mol_therm_opt");
  strcpy(dict[2],"nmol");          strcpy(dict[3],"mol_index");

  spec_root->nmol  = spec_root->molidx = spec_root->napm  = 0;
  spec_root->nbond = spec_root->nbend  = spec_root->ntors = 0;
  spec_root->n14   = spec_root->nbondx = spec_root->npath = 0;
  spec_root->next  = NULL;

  ispec = 0;
 
  /* open setfile */
  if ((fp=fopen(setfile,"r"))==NULL){
    sprintf(line,"can't open set file \"%s\"",setfile);
    md_error(line);
  } 

  sprintf(line,"Reading in set from file \"%s\"",setfile);
  md_stdout(line);

  /* loop until end of file */
  nline = nch = ispec = 0;
  while( (ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      get_meta_key(fp,setfile,line,&nch,&nline,metakey,&key_root,&num_keys);

#ifdef DEBUG
      md_stdout(metakey);
      for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
        sprintf(line,"%d %s %s",i,key_now->keyword,key_now->keyarg);
        md_stdout(line);
      }
#endif
      if(!strcasecmp(metakey,MOL_DEF)){
        for(i=0;i<num_dict;i++) num_found[i] = 0;
        for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
          icase = get_dict_num(num_dict,dict,key_now);
          if(icase == -1){
            print_dict(key_now->keyword,num_dict,dict);
          }
          num_found[icase]++;
          switch (icase) {
             case 0:strcpy(spec_now->filename,key_now->keyarg); /*mol_parm_file*/
               break;
             case 1:strcpy(spec_now->thermopt,key_now->keyarg); /*mol_therm_opt*/
               break;
             case 2:sscanf(key_now->keyarg,"%d",&(spec_now->nmol)); /*nmol*/
               break;
             case 3:sscanf(key_now->keyarg,"%d",&(spec_now->molidx));/*mol_index*/
               break;
             default:
               print_dict(key_now->keyword,num_dict,dict); break;
          }
        }
        for(i=0;i<num_dict;i++){
          if(num_found[i] != 1){
            err_found(dict[i],num_found[i],setfile,nline);
          }
        }
        ++ispec;
        spec_now->next = (SPECIES *)malloc(sizeof(SPECIES));
        spec_now = spec_now->next;
        spec_now->nmol  = spec_now->molidx = spec_now->napm = 0;
        spec_now->nbond = spec_now->nbend  = spec_now->ntors = 0;
        spec_now->n14   = spec_now->nbondx = spec_now->npath = 0;
        spec_now->next = NULL;
      }
      free_key(num_keys,&key_root);
      num_keys=0;
    } /* if meta_key */
    if(ich == NEWLINE){nch = 0; nline++;}
  }
  fclose(fp);

#ifdef DEBUG
  sprintf(line,"species = %d",ispec);
  md_stdout(line);

  for(i=0,spec_now=spec_root;i<ispec;i++,spec_now = spec_now->next){
    sprintf(line,"%s %s %d %d",spec_now->filename,spec_now->thermopt,
	   spec_now->nmol,spec_now->molidx);
    md_stdout(line);
  }
#endif

  *nspec = ispec;
  if(*nspec <= 0 ) {
    md_error("you must specify at least one species in the setfile");
  }
  free_key(num_keys,&key_root);
}

/*------------------------------------------------------------*/

