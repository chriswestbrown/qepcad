#ifndef _SLFQ_SIMPLIFY_
#define _SLFQ_SIMPLIFY_
#include "polynom.h"

/***************************************************************
 *** Prototype declarations for other functions from this package.
 ***************************************************************/
class Context
{
public:
  int qesimp_count, qecallslen, var_num, cutoff;
  bool fdflag;       // full dimensional cells flag
  bool pflag;        // Print out original formula
  bool aflag;        // Assumption flag
  bool quiet;        // quiet mode flag
  bool qnflag;       // qepcad normalize flag
  bool uservarf;     // the user defined a variable ordering
  bool sigintinfo;   // print info on a single sigint
  vector<string> VV; // vector of variable names
  string aname;      // Assumptions file name
  string aform;      // Assumptions formula
  string ofname;     // Output file name
  string sfname;     // "Save-input" file name
  string ifname;     // input file name ("" means stdin)
  string SGCASize ;  // Saclib garbage collected array size 
                     // Number of cells (in millions)
  string SLPLSize ;  // Saclib Long prime list size
  Context() { qecallslen = qesimp_count = 0; cutoff = 2; 
              fdflag = pflag = aflag = quiet = qnflag = uservarf = sigintinfo = false; 
              SGCASize = "5"; SLPLSize = "2000"; }
};

void init(Context &C);
void cleanup(Context &C);
fpart* qepcaddsimplify(logop* root, Context &C);
fpart* simplify(fpart* root, Context &C, int infflag=0);
fpart* qepcadNormalize(fpart* root, Context &C);
vector<string> chooseVarOrder(fpart *p, Context &CC);
extern ostringstream* curr_form; // Points to formula QEPCADB is
                                 // currently simplifying. Is 0 if
                                 // not in a QEPCADB call.
#endif
