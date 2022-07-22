/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "initValues.h++"
#include "lighting.h"

#define QUALITY 0
#define BOND_RADIUS .1

/*--------------------------------------------------------------------*/
void initSys(SYSTEM &system)
{
  system.nconf = system.natoms = system.nvectors = 0;
  system.npolygons = system.nribbons = system.iskip = 0;
  system.idisp = NULL;
  system.dt = 0;
  system.atoms = NULL;
  system.bonds = NULL;
  system.vectors = NULL;
  system.hbonds = NULL;
  //system.polygons = NULL;
  //system.ribbons = NULL;
  system.cell = NULL;
  system.jscell = NULL;
  system.coords = NULL;
}

/*--------------------------------------------------------------------*/
void usage(char *command)
{
  fprintf(stderr,"USAGE: %s [-h] [-n #] [-{p:j:x} filename] [-a filename]",
	  command);
  fprintf(stderr," [setfile] [configsfile]\n");
  exit(1);
}

/*--------------------------------------------------------------------*/
void initSetValues(int argc,char *argv[],SETVALUES &sv)
{
  char *command=argv[0];
  
  sv.frame = 0;
  sv.setfile = sv.confile = sv.attfile = NULL;
  sv.movie_type=0;
  sv.disp_list=0;
  sv.iatom=0;
  sv.skip=0;
  sv.background[0] = 0.;
  sv.background[1] = 0.;
  sv.background[2] = 0.;
  sv.background[3] = 1.;
  sv.sphere_quality = QUALITY;
  sv.cylinder_quality = QUALITY+2;
  sv.bond_radius = BOND_RADIUS;
  sv.sphere_scale = 1.;
  sv.line_width = 1.;
  sv.gamma = 1.;

  while(argc > 1){
    if((*++argv)[0] == '-'){
      --argc;
      while(*(++argv[0]) != '\0'){
	switch(*(argv[0])){
	case 'n':
	  sscanf(*(++argv),"%d",&sv.frame);
	  --argc; *(++argv[0]) = '\0';--argv[0];
	  break;
	case 'j':
	  sv.movie_type=1;
	  sv.setfile = strdup(*(++argv));
	  --argc; *(++argv[0]) = '\0';--argv[0];
	  break;
	case 'p':
	  sv.movie_type=2;
	  sv.setfile = strdup(*(++argv));
	  --argc; *(++argv[0]) = '\0';--argv[0];
	  break;
	case 'x':
	  sv.movie_type=3;
	  sv.setfile = strdup(*(++argv));
	  --argc; *(++argv[0]) = '\0';--argv[0];
	  break;
	case 's':
	  sscanf(*(++argv),"%d",&(sv.skip));
          --argc; *(++argv[0]) = (char )NULL;--argv[0];
	  break;
	case 'a':
	  if (sv.attfile == NULL){
	    sv.attfile = strdup(*(++argv));
	    --argc; *(++argv[0]) = '\0';--argv[0];
	  } else {
	    fprintf(stderr,"Attribute flag set twice!\n");
	    usage(command);
	  }
	  break;
	case 'h':
	  fprintf(stdout,"\nProgram %s ",command);
	  fprintf(stdout,"displays a movie file from the position\n");
	  fprintf(stdout,"configuration file generated with md\n\n");
	  fprintf(stdout,"-h (display's this screen)\n");
	  fprintf(stdout,"-j John Shelly movie format\n");
	  fprintf(stdout,"-p Protein DataBase format\n");
	  fprintf(stdout,"-x XYZ format\n");
	  fprintf(stdout,"-a attribute file \n");
	  fprintf(stdout,"-s # skip # configures per frame \n");
	  fprintf(stdout,"-n where # is the frame number (default = %d\n",
		  sv.frame);
	  usage(command);
	  break;
	default:
	  usage(command);
	  break;
	}
      }
    } else if (sv.setfile == NULL){
      sv.setfile = strdup(argv[0]);
      --argc;
    } else if (sv.confile == NULL) {
      sv.confile = strdup(argv[0]);
      --argc;
    } else {
      usage(command);
    }
  }
}
/*--------------------------------------------------------------------*/
void initGL(SETVALUES &sv)
{
  GLfloat mat_ambient[] = {1.0,1.0,1.0,1.0};
  GLfloat mat_diffuse[] = {1.0,1.0,1.0,1.0};
  GLfloat mat_specular[] = {1.0,1.0,1.0,1.0};
  GLfloat shininess[] = {50.};
  GLfloat mat_emission[] = {0.0,0.0,0.0,1.0};

  glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
  glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
  glMaterialfv(GL_FRONT,GL_EMISSION,mat_emission);
  glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
  glMaterialfv(GL_FRONT,GL_SHININESS,shininess);

  glColorMaterial(GL_FRONT,GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  /* glEnable (GL_LINE_SMOOTH);  */

  /*  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);
#ifdef CULL_FACE
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
#endif

  /* initialize some stuff related to the line width */
  glGetFloatv(GL_LINE_WIDTH_GRANULARITY,sv.line_range);
  sv.line_granularity=sv.line_range[0];
  glGetFloatv(GL_LINE_WIDTH_RANGE,sv.line_range);
#ifdef DEBUG
  printf("line width = %lg, ",sv.line_width);
  printf("granularity = %lg, ",sv.line_granularity);
  printf("range = %lg %lg \n",sv.line_range[0],sv.line_range[1]);
#endif
  
  light_init();  
}

/*--------------------------------------------------------------------*/
