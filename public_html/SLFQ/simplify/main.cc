/***************************************************************
 ** SLFQ: Simplifying Large Formulas with QEPCAD
 **
 ** This is the main file for SLFQ.  It uses poly.l & poly.y to
 ** parse formulas, polynom.h to provide a parse-tree data
 ** structure, and search.cc to excise the solution formula from
 ** a QEPCAD output file.
 ***************************************************************/
#include "polynom.h"
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"
#include "simplify.h"
#include "option.h"
#include "slfqargs.h"

const string programDescription = "\
SLFQ - Simplifying Large Formulas with QEPCADB.\n\
\n\
slfq [ options ]\n\
\n\
Input formula syntax:\n\
The input for slqf is a quantifier-free formula in either \
QEPCAD B or Redlog format.  Formulas are restricted to \
and's and or's of atomic formulas with right-hand-side \
of zero (or an acceptable indexed root expression in \
QEPCAD B's input format.).  Polynomials in atomic \
formula *must* be in expanded form - i.e. no ()'s \
allowed.  Rational numbers, e.g. 3/4, are allowed, as \
well as division within a term by a positive constant, \
e.g. 3 x/10 y + 1 > 0. Both [ ]'s and ( )'s can be used \
for grouping.\n";

/***************************************************************
 *** Main 
 ***************************************************************/
int main(int argc, char **argv)
{
  //-- Initialize & Process Command Line Options --------------//
  Context C;
  ProcArgs<Context> A(programDescription,"-h");
  A.add(new FullDimension);
  A.add(new VarList);
  A.add(new CutOff);
  A.add(new SaveInput);
  A.add(new Version);
  A.add(new Quiet);
  A.add(new OutputFile);
  A.add(new Assumption);
  A.add(new AssumeFile);
  A.add(new SACLIB_N_Opt);
  A.add(new SACLIB_L_Opt);
  A.add(new QEPCAD_NORM_Opt);
  A.add(new Info_on_interrupt);
  A.add(new InputFile);
  if (A.process(argc,argv,C) != 0) exit(1);

  //-- Initialize QEPCADB & SACLIB ---------------------------//
  init(C);

  //-- Read and parse input.
  fpart* p = parsefile(C.ifname);
  if (p == 0) {
    cerr << "Input formula could not be parsed!" << endl;
    exit(1); }
  set<string> S = p->varset();
  C.var_num = S.size();


  //--  Create default variable order if none is given
  if (C.VV.size() == 0) { 
    for(set<string>::iterator i = S.begin(); i != S.end(); ++i)
      C.VV.push_back(*i); }

  //-- Check that given variable order matches variables in formula
  else {
    bool eqf = true;
    for(set<string>::iterator j = S.begin(); eqf && j != S.end(); ++j)
    {
      vector<string>::iterator k = C.VV.begin();
      while(k != C.VV.end() && (*k) != (*j)) ++k;
      eqf = eqf && k != C.VV.end(); 
    }
    if (!eqf) { 
      cerr << "Variables in order do not match variables in formula!" << endl;
      exit(1);
    }
  }

  //-- Use QEPCAD to normalize the input formula by factoring, etc. --//
  if (C.qnflag)
  {
    fpart *np = qepcadNormalize(p,C);
    delete p;
    p = np;
  }
  

  //-- Deal with assumptions (if any!)
  if (C.aform != "")
  {
    // Check that assumptions variables are part of order
    fpart *aroot = parsestring(C.aform.c_str());
    set<string> aS = aroot->varset();
    bool eqf = true;
    for(set<string>::iterator j = aS.begin(); eqf && j != aS.end(); ++j)
    {
      vector<string>::iterator k = C.VV.begin();
      while(k != C.VV.end() && (*k) != (*j)) ++k;
      eqf = eqf && k != C.VV.end(); 
    }
    if (!eqf) { 
      cerr << "Assumptions variables do not match variables in formula/order!"
	   << endl;
      exit(1);
    }
    delete aroot;
  }

  //-- Select an order if none given
  if (!C.uservarf)
    C.VV = chooseVarOrder(p,C);

  //-- Print out info about what's been read.
  if (!C.quiet)
  {
    cout << "The input formula is: ";
    p->report(cout);
    cout << endl;
    cout << "It contains " << p->atomnum() << " atomic formulas, "
	 << "nested to a depth of " << p->depth() << "." << endl;
    cout << "Variables are: ";
    for(vector<string>::iterator i = C.VV.begin(); i != C.VV.end(); ++i)
    {
      if (i != C.VV.begin()) cout << ',';
      cout << *i;
    }
    cout << endl;
    cout << "Cutoff is " << C.cutoff << endl;
  }

  //-- Print out original formula in QEPCAD syntax if requested --//
  if (C.pflag)
  {
    ofstream ofs(C.sfname.c_str());
    p->write(ofs);
  }

  //-- Do the simplification!
  fpart *q = simplify(p,C,1);
  if (!C.quiet) cout << endl;
  // RIGHT HERE I SHOULD CONVERT TO TARSKI IF THAT'S REQUIRED
  cleanup(C);

  //-- Write the output formula
  if (C.ofname != "") 
  { ofstream fout(C.ofname.c_str()); q->write(fout); fout << endl; }
  else 
  {     
    if (!C.quiet) { cout << "An equivalent formula is:" << endl; }
    q->write(cout); cout << endl; 
  }

  //-- Stats
  if (!C.quiet) 
  {
    cout << "There were " << C.qesimp_count << " QEPCADB calls!" << endl;
  }

  return 0;
}
