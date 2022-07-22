/* 
   Copyright 1997 Preston Moore and University of Pennsylvania
*/

#ifndef GET_MOLS_H_INCLUDED

#include <stdio.h>
#include "typedefs.h"

#define META_KEY_CHAR '~'
#define NEWLINE '\n'
#define MOL_DEF "mol_def"

void get_species(FILE *fp,SPECIES *spec_now);
void get_atom(FILE *fp,char *filename,int napm,ATOM_TOPOL *atom_topol);
void get_bond(FILE *fp,char *filename,int nbond,int *ibo,int *jbo,WORD *tbo);
void get_bend(FILE *fp,char *filename,int nbend,int *ibe,int *jbe,int *kbe,
   WORD *tbe);
void get_tors(FILE *fp,char *filename,int ntors,int *ito,int *jto,
   int *kto,int *lto,WORD *tto);
void get_onefour(FILE *fp,char *filename,int nof,int *iof,int *jof);
void get_bondx(FILE *fp,char *filename,int nbondx,
   int *ibondx,int *jbondx,int *kbondx,WORD *tbondx);
void get_meta_key(FILE *fp,char *filename,char *line,int *nch,int *nline,
   char *metakey,KEY *key_root,int *num_keys);
void get_keys(FILE *fp,char *filename,char *line,int *nline,int *nch,
   int *num_keys,KEY *key_root);
int getnxtint (char *charinfo);
void get_sim_keys(char *command,char *infile,int *num_keys,KEY *key_root);
int get_dict_num(int num_dict,WORD dict[],KEY *key_now);
void get_species(FILE *fp,SPECIES *spec_now);
void get_atom(FILE *fp,char *filename,int napm,ATOM_TOPOL *atom_topol);
void get_bond(FILE *fp,char *filename,int nbond,int *ibo,int *jbo,WORD *tbo);
void print_dict(WORD keyword,int num,WORD dict[]);
void free_key(int num_keys,KEY *key_now);
void err_found(WORD dict,int num_found,char *filename,int nline);
void syntax_error(char *infile,FILE *fp,int nline,char *line,int nch);

#define GET_MOLS_H_INCLUDED
#endif
