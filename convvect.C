// code to convert old style vectors to new 

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

const int MAXLINE=2048;

void usage(char *s)
{
  cerr << "Usage: " << s << endl;
  cerr << s << " reads in from standard input and pipes to standard output\n";
  cerr << "converts vectors to atom vector format\n";
  exit(1);
}
int main(int argc,char *argv[])
{
  string line;
  int num;
  char ch;

  if(argc > 1){
    usage(argv[0]);
  }
  // read in first line;
  while(cin.get(ch)){
    line += ch;
    if(ch == '\n') break;
  }

  istringstream is(line);
  is >> ch >> num;

  ch = '\n';
  // cin.ignore(MAXLINE,'\n');

  while(ch != EOF){
    if(ch == '#'){
      line.erase();
      while(cin.get(ch)){
	line += ch;
	if(ch == '\n') break;
      }
    }
    cout << line;
    for(int i=0;i<num;++i){
      float a,b,c,d,e,f;
      cin >> a >> b >> c >> d >> e >> f;
      cin.ignore(MAXLINE,'\n');
      cout << d << " " << e << " " << f << endl;
    }
    ch = cin.peek();
    // cout << "ch = " << ch << "\t";
  }
}
