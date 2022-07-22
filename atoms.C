
#include <GL/gl.h>
#include "atoms.h++"
	
#include <list>

/*! 
  constructor
*/
ATOMS :: ATOMS(void)
{
  _ellipse=(float *)NULL;
  _atomName=_groupName=_moleName=_specName=(char *)NULL;
  reset();
  /* set default ellipse */
  // for(int j=0;j<16;j++) ellipse[j] = 0.;
  // ellipse[0] = ellipse[5] = ellipse[10] = ellipse[15] = 1.;
}

/*!
   reset all internal variables
*/
void ATOMS :: reset(void)
{
  _ishow = false;
  _itype=_ispec=_imole=_iatom=_igroup=-1;
  _color[0] = _color[1] = _color[2] = 0; // rgb
  _color[3] = 1.; // alpha
  _radius = _vdw = _amass = _qch = 0.;
  _beta = 1.;
  if(_ellipse != (float *)NULL){  free(_ellipse);}
  if(_atomName != (char *) NULL){ free(_atomName);}
  if(_groupName != (char *) NULL){free(_groupName);}
  if(_moleName != (char *) NULL){ free(_moleName);}
  if(_specName != (char *) NULL){ free(_specName);}
}
/*! 
   destructor
*/
ATOMS :: ~ATOMS(void)
{
  reset();
}

/*! 
  assignment operator 
*/
ATOMS & ATOMS::operator = (const ATOMS &src){ 
  if(this != &src){ // don't assign to ourselves
    reset();
    _itype = src._itype;  _ispec = src._ispec;  _imole = src._imole;
    _iatom = src._iatom;  _igroup= src._igroup; _ishow = src._ishow;
    for(int i=0;i<(DIM+1);i++) _color[i] = src._color[i];
    _radius= src._radius; _vdw   = src._vdw;    _amass = src._amass;
    _qch   = src._qch;
    if(src._ellipse != NULL) {
      _ellipse = new float[16];
      memcpy(_ellipse,src._ellipse,16*sizeof(float));
    }
    if(src._atomName != (char *)NULL){_atomName = strdup(src._atomName);}
    if(src._groupName!= (char *)NULL){_groupName= strdup(src._groupName);}
    if(src._moleName != (char *)NULL){_moleName = strdup(src._moleName);}
    if(src._specName != (char *)NULL){_specName = strdup(src._specName);}
  }
  return *this;
}

/*! 
   set the ellipse matrix
*/
void ATOMS::setellipse(float ell[16])
{
  int i;
  if(_ellipse == NULL) _ellipse = new float[16];
  for(i=0;i<16;i++) _ellipse[i] = ell[i];
}

/*!
  display atom to gl window
*/
void ATOMS::display(float x[],float xoff[],float scale)
{
  glPushMatrix();
  if(_color[3] >= 1.) {
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
  } else {
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
  }
  glTranslatef(x[0]-xoff[0],x[1]-xoff[1],x[2]-xoff[2]);
  if(_ellipse != NULL) glMultMatrixf(_ellipse);
  glColor4fv(_color);
  // this->print(stdout);
  glPushMatrix();
  float ats = _radius*scale;
  glScalef(ats,ats,ats);
  /*	glDrawArraysEXT(GL_TRIANGLES,0,20);*/
  /*sphere(sv.sphere_quality,sv.solid);*/
  glCallList(1);
  glPopMatrix();
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
  glPopMatrix();
}


void sphere(int,int); /* should be put in glsphere.h */
void displayAtoms(int nconf,int natoms,ATOMS *atoms,COORDS *pos,SETVALUES &sv)
{
  static int istart=0;
  int i;
  float* x = pos[nconf].getx();
  float* f = pos[nconf].getf();
  float gamma = sv.gamma;

  glPushMatrix();
// #ifdef TRANS
//   glDepthMask(GL_FALSE);
//   glEnable(GL_BLEND);
//   glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA); 
// #else
//   glDisable(GL_BLEND);
//   glDepthMask(GL_TRUE);
// #endif
  /*  glDisable(GL_LIGHTING);*/
  if(sv.solid) glShadeModel(GL_SMOOTH);
  else glShadeModel(GL_FLAT);
  
  /*  if(istart==0){
      float *buf;
      istart=1;
      buf = sphere_tris(0);
      glVertexPointerEXT(buf[0],GL_FLOAT,3,0,20);
      glNormalPointerEXT(buf[0],GL_FLOAT,3,0,20);
      glEnable(GL_VERTEX_ARRAY_EXT);
      glEnable(GL_NORMAL_ARRAY_EXT);
      }
      */
  if(istart==0){
    istart=1;
    glNewList(1,GL_COMPILE);
    sphere(sv.sphere_quality,sv.solid);
    glEndList();
  }
  // sort atoms according to depth
  float rot[3] = {sv.rot_mat[2],sv.rot_mat[6],sv.rot_mat[10]};
  //float rot[3] = {sv.rot_mat[8],sv.rot_mat[8],sv.rot_mat[10]};
  std::list<std::pair<float,int> > lp;
  for(i=0;i<natoms;i++){
    if(atoms[i].ishow()){
      double alpha = gamma*f[i] + (1.-gamma)*atoms[i].beta();
      // printf("alpha = %f, gamma = %f, f[%d] = %f, beta= %f\n",alpha,gamma,i,f[i],atoms[i].beta());
      atoms[i].acolor(alpha);
      float z = 0;
      for(int k=0;k<DIM;k++) z += rot[k]*x[i*DIM+k];
      lp.push_back(std::pair<float,int>(z,i));      
    }
  }
  lp.sort();  
  std::list<std::pair<float,int> >::iterator lpi;
  for(lpi = lp.begin();lpi != lp.end(); ++lpi){
    i = (*lpi).second;
    atoms[i].display(&x[i*DIM],sv.xoff,sv.sphere_scale);
  }
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
  /*  glEnable(GL_LIGHTING);*/ 
  glPopMatrix();
}

void ATOMS::printid(FILE *out){
  fprintf(out,"%s, %s(%d) %s(%d) %s(%d),type =%d",
	  _atomName,_specName,_ispec,
	  _moleName,_imole,_groupName,_igroup,_itype);
}

void ATOMS::print(FILE *out=stdout){
  printid(out);
  fprintf(out,", ishow = %d, colors = [%f,%f,%f,%f], rad = %f, vdw = %f\n",
	  _ishow,_color[0],_color[1],_color[2],_color[3],_radius,_vdw);
  fprintf(out,"ellipse : ");
  if(_ellipse != NULL)
    for(int i=0;i<16;i++) fprintf(out,"%g ",_ellipse[i]);
  fprintf(out,"\n");
  fprintf(out,"mass = %g, charge = %g\n",_amass,_qch);
  fflush(out);
}

#define RED    0.5
#define GREEN  0.5
#define BLUE   0.5
#define ALPHA  .80
#define RADIUS 0.5
#define VDW    1.0

/*! 
  set default attributes
*/
void ATOMS::setdefatt()
{
  if(_atomName != NULL){ // set type
    if(_atomName[0] != '\0'){
      switch(_atomName[0]){
      case 'H': case 'h': _itype = 1; break;
      case 'C': case 'c': _itype = 6; break;
      case 'N': case 'n': _itype = 7; break;
      case 'O': case 'o': _itype = 8; break;
      case 'F': case 'f': _itype = 8; break;
      case 'P': case 'p': _itype =15; break;
      case 'S': case 's': _itype =16; break;
      case 'X': case 'x': _itype =54; break;
      case 'L': case 'l': _itype =99; break;
      }
    }
  }

  switch(_itype){ 
  case 1: // Hydrogen
    _color[0] = 1.; _color[1] = 1.; _color[2] = 1.; _color[3] = .5;
    _radius= .16;  _vdw = _radius*2.;
    if(_atomName == NULL) _atomName = strdup("H");
    _amass= 1.;
    break;
  case 2: // Helium
    _color[0] = 0.8; _color[1] = 0.8; _color[2]= 0.8; _color[3] = 1.;
    _radius= .465;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("He");
    _amass= 2.;
    break;
  case 6: // Carbon
    _color[0] = 0.15; _color[1] = 0.85; _color[2] = 0.15; _color[3] = 1.;
    _radius= .35;  _vdw= .9;
    if(_atomName == NULL) _atomName= strdup("C");
    _amass= 12.;
    break;
  case 7: // Nitrogen
    _color[0] = .2; _color[1] = .6; _color[2] = 1.; _color[3] = 1.;
    _radius= .375;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("N");
    _amass= 14.;
    break;
  case 8: // Oxygen
    _color[0] = .7; _color[1] = .3; _color[2]= .3; _color[3] = 1.;
    _radius= .365; _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("O");
    _amass= 16.;
    break;
  case 9: // Florine
    _color[0] = 0.98; _color[1] = 0.6; _color[2] = 0.6; _color[3] = 1.;
    _radius= .36;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("F");
    _amass= 19.;
    break;
  case 10: // Neon
    _color[0] = .65; _color[1] = .65; _color[2] = .65; _color[3] = 1.;
    _radius= .355;   _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("Ne");
    _amass= 20.;
    break;
  case 11: // Sodium
    _color[0] = .55; _color[1] = .55; _color[2]= .0; _color[3] = 1.;
    _radius= .75; _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("Na");
    _amass= 23.;
    break;
  case 13: // Aluminium
    _color[0] = .1; _color[1] = .1; _color[2]= .4; _color[3] = 1.;
    _radius= .59;   _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("Al");
    _amass= 27.;
    break;
  case 14: // Silicon
    _color[0] = .3; _color[1] = .3; _color[2]= .3; _color[3] = 1.;
    _radius= .555;
    _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("Si");
    _amass= 28.;
    break;
  case 15: // Phosphorous
    _color[0] = .4; _color[1] = .2; _color[2] = .8; _color[3] = 1.;
    _radius= .555;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("P");
    _amass= 28.;
    break;
  case 16: // Sulfur
    _color[0] = 1.; _color[1] = 1.; _color[2] = 0.; _color[3] = 1.;
    _radius= 0.51;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("S");
    _amass= 32.;
    break;
  case 17: // Clorine
    _color[0] = .1; _color[1] = .95; _color[2] = .1; _color[3] = 1.;
    _radius= .495;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("Cl");
    _amass= 35.45;
    break;
  case 18: // Argon
    _color[0] = .45; _color[1] = .45; _color[2] = .45; _color[3] = 1.;
    _radius= .46;    _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("Ar");
    _amass= 40.;
    break;
  case 19: // Potasium
    _color[0] = .95; _color[1] = .95; _color[2] = .15; _color[3] = 1.;
    _radius= .445;   _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("K");
    _amass= 40.;
    break;
  case 32: // Germanium
    _color[0] = 0.;  _color[1] = 1.; _color[2] = 0.; _color[3] = 1.;
    _radius= .445;   _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("Ge");
    _amass= 72.6;
    break;
  case 36: // Krypton
    _color[0] = 0.; _color[1] = 1.; _color[2] = 0.; _color[3] = 1.;
    _radius= .445;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("Kr");
    _amass= 84.;
    break;
  case 54: // Xenon
    _color[0] = .3; _color[1] = .3; _color[2] = .3; _color[3] = 1.;
    _radius= .655;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("Xe");
    _amass= 131.3;
    break;
  case 100: // type 100
    _color[0] = .8; _color[1] = .3; _color[2] = .25; _color[3] = 1.;
    _radius= .655;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("100");
    break;
  case 108: // type 108
    _color[0] = .1; _color[1] = 1.; _color[2] = .1; _color[3] = 1.;
    _radius= .03;   _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("108");
    break;
  case 109: // type 109
    _color[0] = .1; _color[1] = .1; _color[2] = 1.; _color[3] = 1.;
    _radius= .03;   _vdw= _radius*2.;
    if(_atomName == NULL) _atomName = strdup("109");
    break;
  case 99: // type 99
    _color[0] = .5; _color[1] = .5; _color[2] = .5; _color[3] = 1.;
    _radius= .5;  _vdw= _radius*2.;
    if(_atomName == NULL) _atomName= strdup("99");
    break;
  default:
    fprintf(stderr,"ERROR: Attribute for %s (%d) not found set to default\n"
	    ,_atomName,_itype);
    _color[0] = RED; _color[1] = GREEN; _color[2] = BLUE; _color[3] = ALPHA;
    _radius= RADIUS; _vdw= VDW;
    if(_atomName == NULL) _atomName = strdup("Unknown");
    _amass= 1.;
  }
}
