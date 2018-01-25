#ifndef _CL_OPTION_
#define _CL_OPTION_
/******************************************************
 * The CL_OPTIONS package.  CL_OPTIONS is a package for
 * organizing the processing and documentation of
 * a programs command line options.
 ******************************************************/

/******************************************************
 * Class Option
 ******************************************************/
template<class Context_type>
class Option
{
public:
  virtual string syntax() = 0;
  virtual string descrip() = 0;
  virtual bool match(string s) = 0;
  virtual bool process(Context_type &C, char **argv, int argc, int &i) = 0;
};

/******************************************************
 * Class ProcArgs
 ******************************************************/
template<class Context_type>
class ProcArgs
{
 private:
  vector<Option<Context_type>*> OP;
  string progDescrip, helpSwitch;

 public:

  /*** User's Interface ***********************************/
  ProcArgs(string programDescription, string helpFlag="-h"): 
    progDescrip(programDescription), helpSwitch(helpFlag) { }
  void add(Option<Context_type>* p) { OP.push_back(p); }
  int process(int argc, char **argv, Context_type &C); // 0 indicates success, otherwise failure

  /*** Other Member functions *****************************/
  virtual string indent() { return "   "; }
  void writeBlock(const string &s, ostream &out);
  virtual void writeophelps();
  virtual void help();
};

template<class CT>
int ProcArgs<CT>::process(int argc, char **argv, CT &C) // 0 indicates success, otherwise failure
{
  //-- Initialize Command Line Options -----------------//
  bool osf = true;
  for(int i = 1, j; osf && i < argc; )
  {
    if (argv[i] == helpSwitch) { help(); ++i; return 2; }
    else
    {
      for(j = 0; j < OP.size() && !OP[j]->match(argv[i]); ++j);
      if (j >= OP.size()) {
	cerr << "Option \"" << argv[i] << "\" not understood!" << endl; 
	exit(1); }
      else {
	osf = OP[j]->process(C,argv,argc,i); }
    }
  }
  if (!osf) { return 1; }
  else return 0;
}

template<class CT>
void ProcArgs<CT>::writeBlock(const string &s, ostream &out)
{
  int c = 0;
  for(int i = 0, l = 1; i < s.size(); ++i)
  {
    if (c == 0) out << indent();
    if (s[i] == '\n' || (s[i] == ' ' && c >= 60)) {
      out << endl;
      c = 0; l++;
    }
    else
    {
      out << s[i];
	c++;
    }
  }
  if (c != 0) out << endl;
}

template<class CT>
void ProcArgs<CT>::writeophelps()
{
  for(int j = 0; j < OP.size(); ++j)
  {
    cout << OP[j]->syntax() << endl;
    writeBlock(OP[j]->descrip(),cout);
  }
}

template<class CT>  
void ProcArgs<CT>::help()
{
  cout << endl;
  writeBlock(progDescrip,cout);
  cout << endl;
  cout << "-h" << endl << indent() 
       << "Prints information on options." << endl;
  writeophelps();
}


#endif


