/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

/* routines to read in the different arguments for the metakeys in a
   parameter file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "get_mols.h"

#define NUM_METAKEYS 8
WORD dictmetakey[NUM_METAKEYS] = {
  "mol_name_def","atom_def","bond_def","bend_def",
  "torsion_def","onefour_def","bondx_def","path_def"
};

/*---------------------------------------------------------------*/
void get_species(FILE *fp,SPECIES *spec_now)
{
  char line[MAXLINE];
  int i,ich,nline,nch,num_keys,icase;
  int num_dict=8,num_found[8],ifound;
  int napm,nbond,nbend,ntors,n14,nbondx,npath;
  WORD dict[8],metakey;
  KEY key_root,*key_now;

  strcpy(dict[0],"mol_name");  strcpy(dict[1],"natom");
  strcpy(dict[2],"nbond");     strcpy(dict[3],"nbend");
  strcpy(dict[4],"ntors");     strcpy(dict[5],"n14");
  strcpy(dict[6],"nbondx");    strcpy(dict[7],"npath");

  /* loop until end of file */
  rewind(fp);
  napm=nbond=nbend=ntors=n14=nbondx=npath=0;
  nline = nch = 0;
  while((ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      ifound = 0;
      get_meta_key(fp,spec_now->filename,
		   line,&nch,&nline,metakey,&key_root,&num_keys);
      
      if(!strcasecmp(dictmetakey[0],metakey)){
        ifound = 1;
        for(i=0;i<num_dict;i++) num_found[i] = 0;
        for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
          icase = get_dict_num(num_dict,dict,key_now);
          if(icase == -1){
            print_dict(key_now->keyword,num_dict,dict); exit(1);
          }
          num_found[icase]++;
          switch (icase) {
             case 0:strcpy(spec_now->name,key_now->keyarg);break;
             case 1:sscanf(key_now->keyarg,"%d",&(spec_now->napm));break;
             case 2:sscanf(key_now->keyarg,"%d",&(spec_now->nbond));break;
             case 3:sscanf(key_now->keyarg,"%d",&(spec_now->nbend));break;
             case 4:sscanf(key_now->keyarg,"%d",&(spec_now->ntors));break;
             case 5:sscanf(key_now->keyarg,"%d",&(spec_now->n14));break; 
             case 6:sscanf(key_now->keyarg,"%d",&(spec_now->nbondx));break;
             case 7:sscanf(key_now->keyarg,"%d",&(spec_now->npath));break;
             default:
               print_dict(key_now->keyword,num_dict,dict);  exit(1);  break;
          }
        }
      }
      if(!strcasecmp(dictmetakey[1],metakey)) {napm++;  ifound=1;}
      if(!strcasecmp(dictmetakey[2],metakey)) {nbond++; ifound=1;}
      if(!strcasecmp(dictmetakey[3],metakey)) {nbend++; ifound=1;}
      if(!strcasecmp(dictmetakey[4],metakey)) {ntors++; ifound=1;}
      if(!strcasecmp(dictmetakey[5],metakey)) {n14++;   ifound=1;}
      if(!strcasecmp(dictmetakey[6],metakey)) {nbondx++;ifound=1;}
      if(!strcasecmp(dictmetakey[7],metakey)) {npath++;ifound=1;}

      if(!ifound) print_dict(metakey,NUM_METAKEYS,dictmetakey);
      
      /* free link list of keys */
      free_key(num_keys,&key_root);
    }
    if(ich == '\n'){nch = 0;nline++;}
  }

  if(spec_now->napm != napm){
    sprintf(line,"# of atoms specified %d != %d # of atoms found\n\tsetting # of atoms to %d",spec_now->napm,napm,napm);
    md_warning(line);
    spec_now->napm = napm;
  }

  if(spec_now->nbond != nbond){
    sprintf(line,"# of bonds specified %d != %d # of bonds found\n\tsetting # of bond to %d",spec_now->nbond,nbond,nbond);
    md_warning(line);
    spec_now->nbond = nbond;
  }

  if(spec_now->nbend != nbend){
    sprintf(line,"# of bends specified %d != %d # of bends found\n\tsetting # of bend to %d",spec_now->nbend,nbend,nbend);
    md_warning(line);
    spec_now->nbend = nbend;
  }
  
  if(spec_now->ntors != ntors){
    sprintf(line,"# of torsions specified %d != %d # of torsons found\n\tsetting # of torsions to %d",spec_now->ntors,ntors,ntors);
    md_warning(line);
    spec_now->ntors = ntors;
  }

  if(spec_now->n14 != n14){
    sprintf(line,"# of 1-4 specified %d != %d # of 1-4 found\n\tsetting # of 1-4 to %d",spec_now->n14,n14,n14);
    md_warning(line);
    spec_now->n14 = n14;
  }

  if(spec_now->nbondx != nbondx){
    sprintf(line,"# of cross bonds specified %d != %d # of cross bonds found\n\tsetting # of cross bonds to %d",spec_now->nbondx,nbondx,nbondx);
    md_warning(line);
    spec_now->nbondx = nbondx;
  }
  if(spec_now->npath != npath){
    sprintf(line,"# of paths specified %d != %d # of paths found\n\tsetting # of cross bonds to %d",spec_now->npath,npath,npath);
    md_warning(line);
    spec_now->npath = npath;
  }

  rewind(fp);
}
/*-------------------------------------------------------------------------*/

void get_atom(FILE *fp,char *filename,int napm,ATOM_TOPOL *atom_topol)
{
  int i,ich,nline,nch,num_keys,icase;
  int natoms;
  char line[MAXLINE];
  int  num_dict = 9,num_found[9];
  WORD dict[9];
  WORD metakey;
  KEY key_root,*key_now;

  strcpy(dict[0],"atom_typ");  strcpy(dict[1],"atom_ind");
  strcpy(dict[2],"mass");      strcpy(dict[3],"charge");
  strcpy(dict[4],"group");     strcpy(dict[5],"valence");
  strcpy(dict[6],"alpha");     strcpy(dict[7],"res");
  strcpy(dict[8],"cgunit");    


  /* loop until end of file */
  rewind(fp);
  nline = nch = natoms = 0;
  while((ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      get_meta_key(fp,filename,line,&nch,&nline,metakey,&key_root,&num_keys);
      
      if(!strncasecmp(dictmetakey[1],metakey,strlen(dictmetakey[1]))){
        for(i=0;i<num_dict;i++) num_found[i] = 0;
        for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
          icase = get_dict_num(num_dict,dict,key_now);
          if(icase == -1){
            print_dict(key_now->keyword,num_dict,dict);
            exit(1);
          }
          num_found[icase]++;
          switch (icase) {
	  case 0: /* atom_typ */
	    strcpy(atom_topol[natoms].type,key_now->keyarg);
	    if(natoms+1 > napm) {
	      sprintf(line,"Number of atoms defined exceeds %d",napm);
	      md_error(line);
	    }
	    break;               
	  case 1:  /* atom index */
	    sscanf(key_now->keyarg,"%d",&(atom_topol[natoms].atm_idx));
	    break;
	  case 2: /* mass */
	    sscanf(key_now->keyarg,"%g",&(atom_topol[natoms].mass));  
	    break;
	  case 3: /* charge */
	    sscanf(key_now->keyarg,"%g",&(atom_topol[natoms].charge));
	    break;
	  case 4: /* group name */
	    sscanf(key_now->keyarg,"%s",(atom_topol[natoms].group));
	    break;
	  case 5: /* valence (does nothing for now) */
	  case 6: /* alpha   (does nothing for now) */
	  case 7: /* res   (does nothing for now) */
	  case 8: /* cgunit (does nothing for now) */
	    break;
	  default:
	    printf("Warning %s is not a keyword --- ignoring\n",
		   key_now->keyword);
	    print_dict(key_now->keyword,num_dict,dict); 
	    break;
          }
        }
        /* make sure all keys were found */
        for(i=0;i<3;i++){
          if(num_found[i] != 1){
            err_found(dict[i],num_found[i],filename,nline); 
          }
        }
        if(num_found[3] == 0) atom_topol[natoms].charge = 0.;
        if(num_found[4] == 0) atom_topol[natoms].group[0] = '\0';
        
        natoms++;
        if(natoms>napm){
          sprintf(line,"number of atoms found %d > %d specified",natoms,napm);
          md_error(line);
        }
      }
      free_key(num_keys,&key_root);
    }
    if(ich == '\n'){nch = 0;nline++;}
  }
  rewind(fp);
  if(natoms!=napm){
    sprintf(line,"number of atoms found %d != %d specified\n",natoms,napm);
    md_error(line);
  }
}
/*-------------------------------------------------------------------------*/
void get_bond(FILE *fp,char *filename,int nbond,int *ibo,int *jbo,WORD *tbo)
{
  int i,ich,nline,nch,num_keys,icase;
  int nbonds;
  char line[MAXLINE];
  int  num_dict = 4,num_found[4];
  WORD dict[4],metakey;
  KEY key_root,*key_now;

  strcpy(dict[0],"atom1");  strcpy(dict[1],"atom2");
  strcpy(dict[2],"label");  strcpy(dict[3],"cons");

  /* loop until end of file */
  rewind(fp);
  nline = nch = nbonds = 0;
  while( (ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      get_meta_key(fp,filename,line,&nch,&nline,metakey,&key_root,&num_keys);
      if(!strncasecmp(dictmetakey[2],metakey,strlen(dictmetakey[2]))){
        for(i=0;i<num_dict;i++) num_found[i] = 0;
        for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
          icase = get_dict_num(num_dict,dict,key_now);
          if(icase == -1){
            print_dict(key_now->keyword,num_dict,dict);
            exit(1);
          }
          num_found[icase]++;
          switch (icase) {
             case 0:sscanf(key_now->keyarg,"%d",&ibo[nbonds]); break; /*atom1*/
             case 1:sscanf(key_now->keyarg,"%d",&jbo[nbonds]); break; /*atom2*/
             case 2:strcpy(tbo[nbonds],key_now->keyarg);  break;
             case 3:
               sprintf(line,"constraints not implimented yet");
               md_error(line); break;
             default:
               print_dict(key_now->keyword,num_dict,dict);  exit(1); break;
          }
        }
        for(i=0;i<2;i++){
          if(num_found[i] != 1){
            err_found(dict[i],num_found[i],filename,nline); 
          }
        }
        if(num_found[2]==0) strcpy(tbo[nbonds],"");
        nbonds++;
        if(nbonds>nbond){
          sprintf(line,"number of bonds found %d > %d specified",nbonds,nbond);
          md_error(line);
        }
      }
      free_key(num_keys,&key_root);
    }
    if(ich == '\n'){nch = 0;nline++;}
  }
  rewind(fp);
  if(nbonds!=nbond){
    sprintf(line,"number of bonds found %d != %d specified",nbonds,nbond);
    md_error(line);
  }
}
/*-------------------------------------------------------------------------*/
void get_bend(FILE *fp,char *filename,int nbend,int *ibe,int *jbe,int *kbe,
	      WORD *tbe)
{
  int i,ich,nline,nch,num_keys,icase,nbends;
  char line[MAXLINE];
  int  num_dict=4,num_found[4];
  WORD dict[4],metakey;
  KEY key_root,*key_now;

  strcpy(dict[0],"atom1");  strcpy(dict[1],"atom2");
  strcpy(dict[2],"atom3");  strcpy(dict[3],"label");
  /* loop until end of file */
  rewind(fp);
  nline = nch = nbends = 0;
  while((ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      get_meta_key(fp,filename,line,&nch,&nline,metakey,&key_root,&num_keys);
      if(!strncasecmp(dictmetakey[3],metakey,strlen(dictmetakey[3]))){
	for(i=0;i<num_dict;i++) num_found[i] = 0;
	for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
	  icase = get_dict_num(num_dict,dict,key_now);
	  if(icase == -1){
	    print_dict(key_now->keyword,num_dict,dict);
	    exit(1);
	  }
	  num_found[icase]++;
	  switch (icase) {
	  case 0:sscanf(key_now->keyarg,"%d",&ibe[nbends]);break; /*atom1*/
	  case 1:sscanf(key_now->keyarg,"%d",&jbe[nbends]);break; /*atom2*/
	  case 2:sscanf(key_now->keyarg,"%d",&kbe[nbends]);break; /*atom3*/   
	  case 3:strcpy(tbe[nbends],key_now->keyarg); break;
	  default:
	    print_dict(key_now->keyword,num_dict,dict);exit(1); break;
	  }
	}
	for(i=0;i<3;i++){
	  if(num_found[i] != 1){
	    err_found(dict[i],num_found[i],filename,nline); 
	  }
	}
	if(num_found[3]==0) strcpy(tbe[nbends],"");
	nbends++;
	if(nbends>nbend){
	  sprintf(line,"number of bends found %d > %d specified",nbends,nbend);
	  md_error(line);
	}
      }
      free_key(num_keys,&key_root);
    }
    if(ich == '\n'){nch=0;nline++;}
  }
  rewind(fp);
  if(nbends!=nbend){
    sprintf(line,"number of bends found %d != %d specified",nbends,nbend);
    md_error(line);
  }
}
/*-------------------------------------------------------------------------*/
void get_tors(FILE *fp,char *filename,int ntors,int *ito,int *jto,
	      int *kto,int *lto,WORD *tto)
{
  int i,ich,nline,nch,num_keys,icase,ntorsion;
  char line[MAXLINE];
  int  num_dict = 5,num_found[5];
  WORD dict[5],metakey;
  KEY key_root,*key_now;

  strcpy(dict[0],"atom1");    strcpy(dict[1],"atom2");
  strcpy(dict[2],"atom3");    strcpy(dict[3],"atom4");
  strcpy(dict[4],"label");

  /* loop until end of file */
  rewind(fp);
  nline = nch = ntorsion = 0;
  while( (ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      get_meta_key(fp,filename,line,&nch,&nline,metakey,&key_root,&num_keys);
      
      if(!strncasecmp(dictmetakey[4],metakey,strlen(dictmetakey[4]))){
	for(i=0;i<num_dict;i++) num_found[i] = 0;
	for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
	  icase = get_dict_num(num_dict,dict,key_now);
	  if(icase == -1){
	    print_dict(key_now->keyword,num_dict,dict);
	    exit(1);
	  }
	  num_found[icase]++;
	  switch (icase) {
	  case 0:sscanf(key_now->keyarg,"%d",&ito[ntorsion]); break; /*atom1*/
	  case 1:sscanf(key_now->keyarg,"%d",&jto[ntorsion]); break; /*atom2*/
	  case 2:sscanf(key_now->keyarg,"%d",&kto[ntorsion]); break; /*atom3*/
	  case 3:sscanf(key_now->keyarg,"%d",&lto[ntorsion]); break; /*atom4*/
	  case 4:strcpy(tto[ntorsion],key_now->keyarg); break;  /*label*/
	  default:
	    print_dict(key_now->keyword,num_dict,dict);  exit(1); break;
	  }
	}
	for(i=0;i<4;i++){
	  if(num_found[i] != 1){
	    err_found(dict[i],num_found[i],filename,nline); 
	  }
	}
	if(num_found[4] == 0) strcpy(tto[ntorsion],"");
	ntorsion++;
	if(ntorsion>ntors){
	  sprintf(line,"number of torsions found %d > %d specified",
		  ntorsion,ntors);
	  md_error(line);
	}
      }
      free_key(num_keys,&key_root);
    }
    if(ich == '\n'){nch=0;nline++;}
  }
  rewind(fp);
  if(ntorsion!=ntors){
    sprintf(line,"number of torsions found %d != %d specified",ntorsion,ntors);
    md_error(line);
  }
}
/*-------------------------------------------------------------------------*/
void get_onefour(FILE *fp,char *filename,int nof,int *iof,int *jof)
{
  int i,ich,nline,nch,num_keys,icase,nofs;
  char line[MAXLINE];
  int  num_dict=2,num_found[2];
  WORD dict[2],metakey;
  KEY key_root,*key_now;
  
  strcpy(dict[0],"atom1");  strcpy(dict[1],"atom2");

  /* loop until end of file */
  rewind(fp);
  nline = nch = nofs = 0;
  while( (ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      get_meta_key(fp,filename,line,&nch,&nline,metakey,&key_root,&num_keys);
      
      if(!strncasecmp(dictmetakey[5],metakey,strlen(dictmetakey[5]))){
	for(i=0;i<num_dict;i++) num_found[i] = 0;
	for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
	  icase = get_dict_num(num_dict,dict,key_now);
	  if(icase == -1){
	    print_dict(key_now->keyword,num_dict,dict);
	    exit(1);
	  }
	  num_found[icase]++;
	  switch (icase) {
	  case 0:sscanf(key_now->keyarg,"%d",&iof[nofs]);break; /*atom1*/
	  case 1:sscanf(key_now->keyarg,"%d",&jof[nofs]);break; /*atom2*/
	  default:
	    print_dict(key_now->keyword,num_dict,dict); exit(1); break;
	  }
	}
	for(i=0;i<2;i++){
	  if(num_found[i] != 1){
	    err_found(dict[i],num_found[i],filename,nline); 
	  }
	}
	nofs++;
	if(nofs>nof){
	  sprintf(line,"number of 1-4s found %d > %d specified",nofs,nof);
	  md_error(line);
	}
      }
      free_key(num_keys,&key_root);
    }
    if(ich == '\n'){nch = 0;nline++;}
  }
  rewind(fp);
  if(nofs!=nof){
    sprintf(line,"number of 1-4s found %d != %d specified",nofs,nof);
    md_error(line);
  }
}
/*---------------------------------------------------------------------*/
void get_bondx(FILE *fp,char *filename,int nbondx,
	       int *ibondx,int *jbondx,int *kbondx,WORD *tbondx)
{
  char line[MAXLINE];
  int i,ich,nline,nch,num_keys,icase,nbondxs;
  int  num_dict=4,num_found[4];
  WORD dict[4],metakey;
  KEY key_root,*key_now;

  strcpy(dict[0],"atom1");  strcpy(dict[1],"atom2");
  strcpy(dict[2],"atom3");  strcpy(dict[3],"label");
  /* loop until end of file */
  rewind(fp);
  nline = nch = nbondxs = 0;
  while((ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if(ich == META_KEY_CHAR){
      get_meta_key(fp,filename,line,&nch,&nline,metakey,&key_root,&num_keys);
      if(!strncasecmp(dictmetakey[6],metakey,strlen(dictmetakey[6]))){
	for(i=0;i<num_dict;i++) num_found[i] = 0;
	for(i=0,key_now=&key_root;i<num_keys;i++,key_now = key_now->next){
	  icase = get_dict_num(num_dict,dict,key_now);
	  if(icase == -1){
	    print_dict(key_now->keyword,num_dict,dict); exit(1);
	  }
	  num_found[icase]++;
	  switch (icase) {
	  case 0:sscanf(key_now->keyarg,"%d",&ibondx[nbondxs]);break;/*atom1*/
	  case 1:sscanf(key_now->keyarg,"%d",&jbondx[nbondxs]);break;/*atom2*/
	  case 2:sscanf(key_now->keyarg,"%d",&kbondx[nbondxs]);break;/*atom3*/
	  case 3:strcpy(tbondx[nbondxs],key_now->keyarg); break;
	  default:
	    print_dict(key_now->keyword,num_dict,dict);exit(1); break;
	  }
	}
	for(i=0;i<3;i++){
	  if(num_found[i] != 1){
	    err_found(dict[i],num_found[i],filename,nline); 
	  }
	}
	if(num_found[3]==0) strcpy(tbondx[nbondxs],"");
	nbondxs++;
	if(nbondxs>nbondx){
	  sprintf(line,"number of cross bonds found %d>%d specified",
		  nbondxs,nbondx);
	  md_error(line);
	}
      }
      free_key(num_keys,&key_root);
    }
    if(ich == '\n'){nch=0;nline++;}
  }
  rewind(fp);
  if(nbondxs!=nbondx){
    sprintf(line,"numbor of cross bonds found %d != %d specified",
	    nbondxs,nbondx);
    md_error(line);
  }
}
/*------------------------------------------------------------------------*/
void get_meta_key(FILE *fp,char *filename,char *line,int *nch,int *nline,
	     char *metakey,KEY *key_root,int *num_keys)
{
  int lenmeta=0,ich;

  /* tilde found loop until [ is found and assign metakey */
  while( (ich = fgetc(fp)) != '[') {
    line[(*nch)++] = (char)ich;
    /* ERROR HANDLING */
    if(ich == '\0' || ich == '~' || ich == '\n' || ich == EOF )
      syntax_error(filename,fp,*nline,line,*nch);

    if(ich != ' ' && ich != '\t'){
      metakey[lenmeta++] = (char)ich;
    }
  }
  if(lenmeta == 0) syntax_error(filename,fp,*nline,line,*nch);
  metakey[lenmeta++] = '\0';

  line[(*nch)++] = (char)ich;
  get_keys(fp,filename,line,nline,nch,num_keys,key_root);
}
/*---------------------------------------------------------*/ 

void get_keys(FILE *fp,char *filename,char *line,int *nline,int *nch,
	 int *num_keys,KEY *key_root)
{
  int ich;
  int lenkey,lenarg;
  KEY *key_now;
  
  *num_keys = 0;
  lenkey = lenarg = 0;
  key_now = key_root;
  key_now->next = NULL;
  strcpy(key_now->keyword,"");
  strcpy(key_now->keyarg,"");

  while( (ich = fgetc(fp)) != ']') {
    line[(*nch)++] = (char)ich;
    
    if((*nch) >= MAXLINE || ich==EOF)
      syntax_error(filename,fp,*nline,line,*nch);
    
    if( ich == '\\' ){
      /* get new key word */
      (*num_keys)++;
      while( (ich = fgetc(fp)) != '{') {
	line[(*nch)++] = (char)ich;
	if( ich == '\0' || ich == '[' || ich == '\\' || ich == '\n')
	  syntax_error(filename,fp,*nline,line,*nch);
	
	if( ich != ' ' && ich != '\t')
	  key_now->keyword[lenkey++] = (char)ich;
      }
      key_now->keyword[lenkey] = '\000';
      line[(*nch)++] = (char)ich;
      /* get new key argument */
      while((ich= fgetc(fp)) != '}' ){
	line[(*nch)++] = (char)ich;
	if( ich == '\0' ||  ich == '[' || ich == '\\' || 
	   ich == '\n' || ich == '{')
	  syntax_error(filename,fp,*nline,line,*nch);
	
	if( ich != ' ' && ich != '\t')
	  key_now->keyarg[lenarg++] = (char)(ich);	
      }
      if(lenarg==0) syntax_error(filename,fp,*nline,line,*nch);
      key_now->keyarg[lenarg] = '\000';
      line[(*nch)++] = (char)ich;

      key_now->next = (KEY *)malloc(sizeof(KEY));
      key_now = key_now->next;  lenkey = 0;
      lenarg = 0;      key_now->next = NULL;

    }
    /* if new line update nline and reset nch */
    if(ich == '\n'){*nch = 0;(*nline)++;}
  }
}
/*------------------------------------------------------------------------*/
void free_key(int num_keys,KEY *key_now)
{
  KEY *key_next;

  /* set over key_root */
  key_now = key_now->next;

  /* loop over allocated keys */
  while(num_keys-->0){
    key_next = key_now->next; free(key_now); key_now = key_next;
  }
}
/*------------------------------------------------------------------------*/

void syntax_error(char *infile,FILE *fp,int nline,char *line,int nch)
{
  int ich;

  fprintf(stderr,"ERROR: Syntax error in file %s at line %d\n",infile,nline);
  for(ich=0;ich<nch;ich++) fprintf(stderr,"%c",line[ich]);
  if(line[--nch] != '\n')  fprintf(stderr,"%c",'\n');
  for(ich=0;ich<nch;ich++) fprintf(stderr,"%c",'_');
  fprintf(stderr,"%c\n\n",'^');
  exit(1);
}
/*---------------------------------------------------------------------*/
int get_dict_num(int num_dict,WORD dict[],KEY *key_now)
{
  int i;
  
  for(i=0;i<num_dict;i++){
    if(!(strcasecmp(dict[i],key_now->keyword))){
      return i;
    }
  }
  return -1;
}
/*-----------------------------------------------------------------------*/ 
void print_dict(WORD keyword,int num,WORD dict[])
{
  int i,j;
  fprintf(stderr,"ERROR: keyword \"%s\" not in the dictionary\n",keyword);
  for(j=0;j<num/2;j++){
    i = j*2;
    if(dict[i][0] != '\0') fprintf(stdout,"dict[%d] = %s\t",i,dict[i]);
    if(dict[i+1][0] != '\0')fprintf(stdout,"\tdict[%d] = %s",i+1,dict[i+1]);
    fprintf(stdout,"\n");
  }
  if(num%2 !=0){
    if(dict[num-1][0] != '\0') 
      fprintf(stdout,"dict[%d] = %s\t",num-1,dict[num-1]);
    fprintf(stdout,"\n");
  }
}
/*-----------------------------------------------------------------------*/ 
void err_found(WORD dict,int num_found,char *filename,int nline)
{
  fprintf(stderr,"ERROR: keyword %s found %d times in file %s at line %d\n",
	  dict,num_found,filename,nline);
  exit(1);
}  
/*-----------------------------------------------------------------------*/ 
void write_key(KEY *key){fprintf(stdout,"%s %s\n",key->keyword,key->keyarg);}
/*-----------------------------------------------------------------------*/ 
void get_sim_keys(char *command,char *infile,int *num_keys,KEY *key_root)
{
  char backslash,nil,newline,lbrace,rbrace,space,tab;
  char line[MAXLINE],string[MAXLINE];
  int nline,nch,ich;
  int lenkey,lenarg;
  FILE *fp;
  KEY *key_now;
  
  key_now = key_root; lenkey = 0; lenarg = 0;
  
  backslash = '\\'; nil = '\0'; newline = '\n'; lbrace = '{';
  rbrace = '}'; space = ' '; tab = '\t';

  nline =  1; nch = 0;
  
  fp = fopen(infile,"r");
  if (fp == NULL){
    sprintf(string,"%s can't open file \"%s\"",command,infile);
    md_error(string);
  }
  
  while( (ich = fgetc(fp)) != EOF) {
    line[nch++] = (char)ich;
    if( ich == backslash){
      (*num_keys)++;      
      while( (ich = fgetc(fp)) != lbrace){
	line[nch++] = (char)ich;
	if(ich == EOF || ich == backslash || ich == newline)
	  syntax_error(infile,fp,nline,line,nch);
	if(ich != space && ich != tab)
	  key_now->keyword[lenkey++] = (char)ich;
      }
      key_now->keyword[lenkey] = nil;
      line[nch++] = (char)ich;
      while( (ich = fgetc(fp)) != rbrace){
	line[nch++] = (char)ich;
	if(ich == EOF || ich == backslash || ich == newline || ich == lbrace)
	  syntax_error(infile,fp,nline,line,nch);
	if(ich != space && ich != tab)
	  key_now->keyarg[lenarg++] = (char)ich; 
      }
      key_now->keyarg[lenarg] = nil;
      line[nch++] = (char)ich;
      key_now->next = (KEY *)malloc(sizeof(KEY));
      key_now = key_now->next;
      lenkey = 0;
      lenarg = 0;
    }
    if(ich == '\n'){nch = 0;nline++;}
  }
  fclose(fp);
}
/*---------------------------------------------------------------------*/ 
int getnxtint (char *charinfo)
{
  static char *charinfoold=NULL;
  static int iloc;
  static int digit;
  int n;

  if(charinfoold != charinfo){
    charinfoold=charinfo;
    iloc = 0;
    digit = 0;
  }

  if(charinfo[iloc] == '\0') return digit;
  if (strcmp(charinfoold,charinfo)==0){
    while(!isdigit(charinfo[iloc]) && (charinfo[iloc] != '-')) {
      if(charinfo[iloc] == '\0') return digit;
      iloc++;
    }
  } else {
    iloc = 0;
    charinfoold = charinfo;
  }
  sscanf(&charinfo[iloc],"%d%n",&digit,&n);

  iloc += n;
  return digit;
}
/*---------------------------------------------------------------------*/ 
