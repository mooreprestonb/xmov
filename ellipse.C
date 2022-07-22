/**********************************************************************/
void read_ellipse(char *filename,int natoms,float **ellipse)
{
  int i,j;
  char line[MAXLINE],name[MAXLINE];
  FILE *fp;
  
  /* open vector file */
  if ( (fp = fopen(filename,"r")) == NULL){
    sprintf(line,"can't open ellipse filename \"%s\"",filename);
    md_warning(line);
    return;
  }
  
  /* read header */
  if(fgets(line,MAXLINE,fp)==NULL){
    sprintf(line,"can't get header of ellipse file %s",filename);
    md_warning(line);
    return;
  }

  if((sscanf(line,"%s %d %s",name,&j,line)!=3) && 
     (strcmp(name,"#")!=0)){
    fprintf(stderr,"ERROR: something is wrong with the header of file %s\n",
	    filename);
    return;
  }
  if(j != natoms ){
    fprintf(stderr,"ERROR: number of ellipsiods %d != %d number of atoms\n",
	    natoms,j);
    return;
  }
  printf("nellipse = %d  (remark: %s)\n",natoms,line);
  for(i=0;i<natoms;i++){
    fscanf(fp,"%g %g %g %g %g %g %g %g %g",
	   &(ellipse[i][0]),&(ellipse[i][1]),&(ellipse[i][2]),
	   &(ellipse[i][4]),&(ellipse[i][5]),&(ellipse[i][6]),
	   &(ellipse[i][8]),&(ellipse[i][9]),&(ellipse[i][10]));
  }
  fgets(line,MAXLINE,fp); /* read to end of line */
  fclose(fp);
  
#ifdef DEBUG
  for(i=0;i<natoms;i++){
    printf("%d %9g %9g %9g %9g %9g %9g %9g %9g %9g\n",i,
	   ellipse[i][0],ellipse[i][1],ellipse[i][2],
	   ellipse[i][4],ellipse[i][5],ellipse[i][6],
	   ellipse[i][8],ellipse[i][9],ellipse[i][10]);
  }
#endif
}
