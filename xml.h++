/*! 
  \file xml.h++
  \brief xml class definition and functions 
  
  these functions help parse up xml files
*/

#ifndef XML_H
#define XML_H

#include <iostream>
#include <list>
#include <string>
#include <algorithm>

class XML {
private:
  std::string _tag; //!< tag string
  std::string _att; //!< attribute string
  std::list<std::string> _data; //!< list of string between start and end tag
  std::list<std::string>::iterator _pos; //!< current pos in list
  std::list<XML> _xmllist; //!< list of sub tags
  // std::list<XML> *_parent;
public:
  //! constructor
  XML(void){
    _tag="";_att="";_data.push_back("");_pos = _data.begin();
    // _parent = 0;
  } 
  XML(const XML &sxml){ *this = sxml;}//!< copy constructor
  XML & operator=(const XML &src){//!< copy assignment
    if(this != &src){ // don't assign to ourselves
      _tag = src._tag;   _att = src._att;
      _data = src._data; _pos = src._pos;
      std::list<std::string>::iterator _post = _data.end(); --_post;
      if(*(_post) == *(_pos)) _pos = _post;
      else // not at end lets go look for it;
	_pos = find(_data.begin(),_data.end(), *(_pos));
      _xmllist = src._xmllist;
    }
    return (*this);
  }
  ~XML(void){} //!< default destructor
  std::string tag(void){return _tag;} //!< return tag string
  void tag(std::string &stag){_tag = stag;} //!< set tag string
  std::string att(void){return  _att;}; //!< return attribute string
  void att(std::string & satt){_att = satt;} //!< set tag string
  std::list<std::string> & data(void){return _data;} //!< return data address
  //!< return address of position iterator
  std::list<std::string>::iterator & pos(void){return _pos;} 
  // void parent(std::list<XML> *parent){_parent = parent;}
  // std::list<XML> * parent(void){return _parent;}
  //! set data string
  void data(const std::string & sdata){(*_pos) = sdata;}
  //! add to data string
  void dataadd(const std::string & sdata){ (*_pos) += sdata;}
  //! push back data string and add new data string
  void dataaddnext(const std::string & sdata){
    _data.push_back(sdata); _pos = _data.end(); --_pos;
  }
  //! return list of subtags
  std::list<XML> & xmllist(void) {return _xmllist;};
  //! return data position iterator
  std::list<std::string>::iterator datapos(void){return _pos;}
  //! return begining data iterator
  std::list<std::string>::iterator databegin(void){return _data.begin();}
  //! return end data iterator
  std::list<std::string>::iterator dataend(void){return _data.end();}
  //! equal operator
  bool operator==(const XML &src){
    if(this == &src) return true;
    if(_tag!=src._tag) return false;
    if(_att!=src._att) return false;
    if(_data!=src._data) return false;
    return true;
  }
  //! not equal operator (full class)
  bool operator!=(const XML &src){
    if(*this == src) return false;
    return true;
  }
  //! not equal operator (checks just tag strings)
  bool operator!=(const std::string &ssrc){
    if(ssrc==_tag) return false;
    return true;
  }
  //! print out the XML class
  friend std::ostream & operator << (std::ostream & stream, XML & src){
    stream << "tag = \"" << src._tag << "\", att = \""<< src._att <<"\"\n";
    {
      int j=-1,i=0;
      std::list<std::string>::iterator lnow,lend;
      lnow = src.databegin(); lend = src.dataend(); 
      for(;lnow != lend; ++lnow,++i){
	stream << "\tstring("<<i<<") = \""<<(*lnow)<<"\"\n";
	if(lnow == src._pos) j=i;
      }
      stream <<"\tpos at = "<< j << std::endl;
      std::list<XML>::iterator lxml;
      lxml = src._xmllist.begin();
      while(lxml!= src._xmllist.end()){stream<<(*lxml);++lxml;}
    }
    return stream;
  }
  //! print out the XML class
  void print(std::ostream & stream=cout){ stream << *this;}
  //! search for tag in subtag list
  XML & findtag(std::string tag) {
    // if(_tag == tag) return *this;
    std::list<XML>::iterator it = _xmllist.begin();
    std::list<XML>::iterator end = _xmllist.end();
    for(;it != end; ++it){
      // std::cout <<"tag now = \""<<(*it).tag()<<"\" tag = \""<<tag<<"\"\n";
      if((*it).tag() == tag) return (*it);
    }
    std::cerr << "\"" << tag << "\" not found\n";
    return  *this;
  }
};

#endif
