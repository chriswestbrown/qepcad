#include "option.h"
#include "simplify.h"
#include "slfq_signal.h"

typedef Option<Context> SLFQOption;

class FullDimension : public SLFQOption
{
public:
  string descrip() { return 
"This flag tells the program to do all its computations with full dimensional cells, meaning that\
 the simplified formula agrees with the original on all but a measure-zero subset of the parameter\
 space."; }
  string syntax() { return "-F"; }
  bool match(string s) { return "-F" == s; }
  bool process(Context &C, char **argv, int argc, int &i) 
  { 
    if (match(argv[i])) { C.fdflag = true; ++i; } 
    return true;
  }
};

class OutputFile : public SLFQOption
{
public:
  string descrip() { return
"This flag tells slfq to write its output to the given file.  In the absence of this switch,\
 slfq's output goes to standard out."; }
  string syntax() { return "-o <file>"; }
  bool match(string s) { return "-o" == s; }
  bool process(Context &C, char **argv, int argc, int &i) 
  {   
    if (match(argv[i])) {
      if (i + 1 >= argc) { cout << "No file name given -o option!" << endl; return false; }
      C.ofname = argv[i+1];
      i += 2; }
    return true;
  }
};

class SaveInput : public SLFQOption
{
public:
  string syntax() { return "-S <file>"; }
  string descrip() { return
"This flag tells the program to print out the original formula in QEPCAD syntax to the\
 given file."; }
  bool match(string s) { return "-S" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      C.pflag = true;
      if (i + 1 >= argc) { cout << "No file given for -S option!" << endl; return false; }
      C.sfname = argv[i+1];
      i += 2;
    }
    return true;
  }  
};

class VarList  : public SLFQOption
{
public:
  string syntax() { return "-L \"<var1>,<var2>,...\""; }
  string descrip() { return
"This option allows the user to specify a variable ordering to be used in simplification. \
 CAD construction is very sensitive to the variable ordering. SLFQ tries to choose a good\
 order for you, but you may be able to find a better order by hand.  The first variable in the\
 given list will correspond to the lowest level, the last to the highest level."; }
  bool match(string s) { return "-L" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      if (i + 1 >= argc) { cout << "No variable list given for -L option!" << endl; return false; }
      string s(argv[i+1]);
      for(int j = 0; j < s.size(); ++j) if (s[j] == ',') s[j] = ' ';
      istringstream sin(s);
      for(string w; sin >> w; C.VV.push_back(w));
      i += 2;
      C.uservarf = true;
    }
    return true;
  }  
};

class CutOff : public SLFQOption
{
public:
  string syntax() { return "-c <num>"; }
  string descrip() { return
"Sets the cutoff - the number of atomic formulas at which QEPCAD will simply be called\
 as opposed to going through further subdivisions."; }
  bool match(string s) { return "-c" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      if (i + 1 >= argc) { cout << "No variable list given for -L option!" << endl; return false; }
      int n = atoi(argv[i+1]);
      if (n < 2) {cout << "Improper cutoff value specified with \"-c\" option!" << endl; return false; }
      C.cutoff = n;
      i += 2;
    }
    return true;
  }  
};

class Version : public SLFQOption
{
public:
  string syntax() { return "-v"; }
  string descrip() { return "Prints the SLFQ version number."; }
  bool match(string s) { return "-v" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      printversion(cout);
      i++;
    }
    return false;
  }  
};

class Quiet : public SLFQOption
{
public:
  string syntax() { return "-q"; }
  string descrip() { return "Runs program in \"quiet\" mode - i.e. without diagnostic info."; }
  bool match(string s) { return "-q" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      C.quiet = true;
      i++;
    }
    return true;
  }  
};



class Assumption : public SLFQOption
{
public:
  string syntax() { return "-a <formula>"; }
  string descrip() { return
"This tells the program to use QEPCAD B's \"assume\" command with the assumption\
 formula given as command line argument.  Example:\n\n\
   slfq example/Ex3 -a \"beta4 > 0 /\\ nu > 0\"\n\n\
This assumption will get passed along to each black box call to QEPCAD B for simplification.."; }
  bool match(string s) { return "-a" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {      
      if (i + 1 >= argc) { cout << "No formula given for -a option!" << endl; return false; }
      fpart* ap = parsestring(argv[i+1]);
      if (ap != 0)
      {
	char *p = treetostring(ap);
	C.aform = p;
	delete ap;
      }
      else
      {
	cout << "Assumption could not be parsed!" << endl;
	return false;
      }
      i += 2;
    }
    return true;
  }  
};

class AssumeFile : public SLFQOption
{
public:
  string syntax() { return "-A <file>"; }
  string descrip() { return
"This file tells the program to use QEPCAD B's \"assume\" command with the assumption formula\
 simply printed verbatim from the given file."; }
  bool match(string s) { return "-A" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      C.aflag = true;
      if (i + 1 >= argc) { cout << "No file given for -A option!" << endl; exit(1); }
      C.aname = argv[i+1];
      ifstream test(C.aname.c_str());
      if (!test) { cout << "Assumptions file \"" << C.aname << "\" not found!" << endl; return false; }      
      i += 2;
    }
    return true;
  }  
};

class InputFile : public SLFQOption
{
  bool fsf; // file set flag
public:
  InputFile() { fsf = false; }
  string syntax() { return "<file>"; }
  string descrip() { return
"Set the file from which slfq reads its input.  If no file is given, input is read\
 from standard in."; }
  bool match(string s) { return !fsf; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      FILE* input = fopen(argv[i],"r"); 
      if (input == NULL) {
	cout << "Input file \"" << argv[i] << "\" not found!" << endl;
	return false; }
      fclose(input);            
      fsf = true;
      C.ifname = argv[i];
      ++i;
    }
    return true;
  }  
};

class SACLIB_N_Opt : public SLFQOption
{
public:
  string syntax() { return "-N <num>"; }
  string descrip() { return
"Set the number of words in SACLIB's garbage collected array to \
<num>*1,000,000. The default value of num is 5, i.e. 5,000,000 words."; }
  bool match(string s) { return "-N" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      if (i + 1 >= argc) { cout << "No value given for -N option!" << endl; exit(1); }
      C.SGCASize = argv[i+1];
      i += 2;
    }
    return true;
  }  
};

class SACLIB_L_Opt : public SLFQOption
{
public:
  string syntax() { return "-P <num>"; }
  string descrip() { return
"Set the number of primes in SACLIB's long primes list.  It is \
equivalent to SACLIB's +L option.  The default is 2000."; }
  bool match(string s) { return "-P" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      if (i + 1 >= argc) { cout << "No value given for -P option!" << endl; exit(1); }
      C.SLPLSize = argv[i+1];
      i += 2;
    }
    return true;
  }  
};

class QEPCAD_NORM_Opt : public SLFQOption
{
public:
  string syntax() { return "-n"; }
  string descrip() { return
"Normalize the input formula with QEPCAD's initializer.  SLFQ already requires a \
formula in a somewhat normalized form (e.g. and/or/not the only operators \"f relop 0\"\
the only inequalities allowed, input polynomials must appear in fully distributed form), \
but this rewrites so that all pol's occurring are irreducible."; }
  bool match(string s) { return "-n" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      C.qnflag = true;
      ++i;
    }
    return true;
  }  
};

/**************************************************************
 ** Info_on_interrupt option
 **************************************************************/
class Info_on_interrupt : public SLFQOption
{
public:
  string syntax() { return "-i"; }
  string descrip() { return
"On signal SIGINT (e.g. ctrl-c from keyboard) information is displayed. If \
two signals are recieved within 1 second slfq terminates. The information \
displayed relates to the current state of slfq's computation."; }
  bool match(string s) { return "-i" == s; }
  bool process(Context &C, char **argv, int argc, int &i)
  {
    if (match(argv[i])) {
      C.sigintinfo = true;
      ++i;
    }
    return true;
  }  

};
