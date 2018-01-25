#include "polynom.h"
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"
#include "simplify.h"
#include "qepcad.h"
#include <cstring>

int yyparse (fpart* &result);

// Definition of global declared in simplify.h
ostringstream* curr_form = 0; 

/**********************************************************
 ** classes & functions for choosing a good variable order.
 **********************************************************/
class vobj
{
public:
  int d, td, nt;
  string v;
  vobj(fpart *p, Context &C, int i) : v(C.VV[i])
  {
    d = p->max_degree(C.VV[i]);
    td = p->max_tdegree(C.VV,i);
    nt = p->num_tcvar(C.VV[i]);
  }
};
bool operator<(const vobj &a, const vobj &b) {
  return 
    a.d < b.d || a.d == b.d && 
    (a.td < b.td || a.td == b.td &&
     (a.nt < b.nt || a.nt == b.nt && a.v < b.v));
}

vector<string> chooseVarOrder(fpart *p, Context &C)
{
  vector<vobj> V;
  for(int i = 0; i < C.VV.size(); ++i)
    V.push_back(vobj(p,C,i));
  sort(V.begin(),V.end());
  vector<string> X;
  for(int i = V.size()-1; i >= 0; --i)
    X.push_back(V[i].v);
  return X;
}


/**********************************************************
 ** QEPCAD interface functions
 **********************************************************/
#include "db/convenientstreams.h"
#include "slfq_signal.h"

void init(Context &C)
{
  int argc = 3,ac;
  char **argv,**av;
  argv = new char*[3];

  //Saclib's +N
  string N = "+N" + C.SGCASize + "000000";
  argv[1] = new char[N.size() + 1];
  strcpy(argv[1],N.c_str());
  
  //Saclib's +L
  string L = "+L" + C.SLPLSize;
  argv[2] = new char[L.size() + 1];
  strcpy(argv[2],L.c_str());

  // These are all the things that need to be done to
  // initialize QEPCAD.
  GVContext = new QEPCADContext;
  ARGSACLIB(argc,argv,&ac,&av);
  BEGINSACLIB((Word *)&argc);
  BEGINQEPCAD(ac,av);
  if (C.sigintinfo)
    init_SIGINT_handler();
  else
    QEPCAD_init_SIGINT_handler();

  delete [] av;
  delete [] argv;
}

Word saclibvarlist(vector<string> &VV)
{
  // Create SACLIB var list
  Word V = NIL;
  for(vector<string>::iterator i = VV.begin(); i != VV.end(); ++i)
    V = COMP(LFS((char*)((*i).c_str())),V);
  V = CINV(V);
  return V;
}

Word string2UnNormForm(const string &S, Word V)
{  
  // Convert from string to QEPCAD Unnormalized Formula
  Word F, t;
  istringstream sin(S + ".\n");

  PushInputContext(sin);
  QFFRDR(V,&F,&t);
  CREAD();
  PopInputContext();

  if (t == 0) 
  { 
    cerr << "QEPCADB could not understand the formula:" << endl
	 << sin.str() << endl;
    exit(1);
  }

  return F;
}

string unNormForm2string(Word F, Word V)
{
  // Convert from Unnormalized QEPCAD formula to string
  ostringstream OTS;
  PushOutputContext(OTS);
  QFFWR(V,F);
  PopOutputContext();
  return OTS.str();
}

/***************************************************************************
 * This function uses QEPCADB's formula normalizer to normalize the formula
 * passed to it.  SLFQ already requires a formula in a somewhat normalized
 * form (e.g. and/or/not the only operators "f relop 0" the only inequalities
 * allowed, input polynomials must appear in fully distributed form).
 ***************************************************************************/
fpart* qepcadNormalize(fpart* root, Context &C)
{
  // Create SACLIB var list
  Word V = saclibvarlist(C.VV);

  // Get formula as a string
  ostringstream os;
  root->write(os);

  // Construct QEPCAD Input formula
  Word F = string2UnNormForm(os.str(),V);
  Word Fs = LIST4(C.VV.size(),C.VV.size(),NIL,F);

  /* Normalize. */
  Word Fh;
  Fh = NORMQFF(F);

  // Translate to string
  string OTS = unNormForm2string(Fh,V);
  
  // Translate trivial true/false 
  if (OTS == "0 = 0") OTS = "TRUE";
  if (OTS == "0 /= 0") OTS = "FALSE";

  return parsestring(OTS.c_str());
}

fpart* qepcaddsimplifyMOD(fpart* root, Context &C)
{
  // Log number of calls to qepcaddsimplify
  C.qesimp_count++;

  // Create SACLIB var list
  Word V = saclibvarlist(C.VV);

  // Get formula as a string
  ostringstream os;
  root->write(os);

  // Construct QEPCAD Input formula
  Word F = string2UnNormForm(os.str(),V);
  Word Fs = LIST4(C.VV.size(),C.VV.size(),NIL,F);

  // Initialize QEPCAD problem
  QepcadCls Q;
  Q.SETINPUTFORMULA(V,Fs);
  
  // Add assumptions if any
  if (C.aform != "")
  {
    Word A = string2UnNormForm(C.aform,V);
    Q.SETASSUMPTIONS(A);
  }

  // Set for full-dimensional cells only (measure-zero error)
  if (C.fdflag)
  {
    Q.PCMZERROR = 1;
  }

  // Create CAD & get simplified equivalent formula
  curr_form = &os;
                    int t1 = ACLOCK();
  Q.CADautoConst();
                    int t2 = ACLOCK();
  if (Q.GVPC == 0) 
    return new logconst((ISATOM(Q.GVNQFF) ? Q.GVNQFF.W : TYPEQFF(Q.GVNQFF)) == TRUE);

  Word Fd = Q.GETDEFININGFORMULA((C.fdflag) ? 'F' : 'E');  
                    int t3 = ACLOCK();
  curr_form = 0;
  
  // Translate to string
  string OTS = unNormForm2string(Fd,V);
  
  // Translate trivial true/false 
  if (OTS == "0 = 0") OTS = "TRUE";
  if (OTS == "0 /= 0") OTS = "FALSE";

  return parsestring(OTS.c_str());
}

void cleanup(Context &C)
{
  if (!C.quiet) {
    SWRITE("\n=====================  The End  =======================\n");
    STATSACLIB(); }
  ENDQEPCAD();
  ENDSACLIB(SAC_FREEMEM);  
}


/***************************************************************
 ***
 root   : a pointer to the root node of a formula expression tree.
 C      : the "Context" for this simplification. 
 infflag: just a flag to tell you if you're at the top level.
 ***
 *** This program returns an fpart formula that is equivalent 
 *** to root (hopefully simpler!). Note:  the returned data
 *** structure is totally distinct from root, and the formula
 *** structure pointed to by root is unchanged by the operation!
 ***
 ***************************************************************/
fpart* simplify(fpart* root, Context &C, int infflag)
{

  /* It's an atom or a constant or already simplified, simply return. */
  if (root->simpleflag > 0)
    return root->copy();

  logop* Root = dynamic_cast<logop*>(root); //-- Root is the logical operator at the root of the tree.
  
  /***************************************************************
   *** Call QEPCAD directly
   ***************************************************************/  
  if (Root == 0)
    return qepcaddsimplifyMOD(root,C);
  if (Root->simpleflag == -1 || Root->atomnum() < C.cutoff)
    return qepcaddsimplifyMOD(Root,C);

  /***************************************************************
   *** Simplify pieces
   ***************************************************************/
  vector<fpart*> simplifiedPieces;
  int N = Root->A.size();     //-- N is the number of operands of root.
  for(int i = 0; i < N; i++) {

    //-- Prints feedback for user --//
    if (infflag && !C.quiet)
      if (Root->isand())
	cout << "Simplifying conjunct number " << i + 1 << endl;
      else if (Root->isor())
	cout << "Simplifying disjunct number " << i + 1 << endl;
    
    // Simplify Root->A[i]
    fpart* p;
    if (Root->A[i]->simpleflag == 0)
      p = simplify(Root->A[i],C);
    else if (Root->A[i]->simpleflag == -1)
      p = qepcaddsimplifyMOD((logop*)(Root->A[i]),C);
    else // simpleflag is 1
      p = Root->A[i]->copy();
    p->simpleflag = 1;

    //-- This chunk just prints feedback for user! --//
    if (infflag && !C.quiet) {
      cout << "Subformula " << i + 1 << ":" << endl;
      p->write(cout);
      cout << endl; }
    else if (!C.quiet) {
      for(int m = 0; m < C.qecallslen; m++) cerr << char(8);
      cerr << C.qesimp_count;
      int k = 1, j = 10;
      while(C.qesimp_count >= j) { k++; j *= 10; }
      if (C.qesimp_count % 10 == 0)
	C.qecallslen = k;
      else
	C.qecallslen = k;
    }
    
    //-- Try quick decision when subformula's a constant. --//
    if (p->is_T_F())
    {
      logconst* q = dynamic_cast<logconst*>(p);
      if (Root->isand() && q->value() == false || Root->isor() &&  q->value() == true)
      {
	for(int j = 0; j < simplifiedPieces.size(); j++)
	  delete simplifiedPieces[j];
	return p;
      }
      else
	delete p;
    }
    else
      //-- replace old subformula with new. --//
      simplifiedPieces.push_back(p);
  }
  
  
  /***************************************************************
   *** Create the conjunction/disjunction of the simplified pieces
   ***************************************************************/
  fpart *q = 0;
  N = simplifiedPieces.size();
  int ac = 0; for(int i = 0; i < N; ++i) ac += simplifiedPieces[i]->atomnum();

  // Empty conjunction/disjunction
  if (N == 0) {
    q = Root->isand() ? new logconst(true) : new logconst(false); 
    q->simpleflag = 1; // Mark as already simplified
  }

  // Conjunction/disjuntion of 1 element
  else if (N == 1) {
    q = simplifiedPieces[0]; 
    q->simpleflag = 1; // Mark as already simplified
  }

  // Conjunction/disjunction of 2 elements or total atomcount <= cutoff
  else if (N == 2 || ac <= C.cutoff) {
    logop *p = Root->isand() ? (logop*) new andop : (logop*) new orop;     
    for(int i = 0; i < N; ++i) p->A.push_back(simplifiedPieces[i]);
    p->simpleflag = -1; // Mark as simplify by direct QEPCAD call
    q = simplify(p,C);
    delete p;
  }

  // Conjunction or disjunction of more than 2 elements with more than cutoff atoms
  else {    
    logop *p = Root->isand() ? (logop*) new andop : (logop*) new orop, *r = 0;
    int j = 0, k = 0;
    while (j < N)
    {
      if (k <= 1 || r->atomnum() + simplifiedPieces[j]->atomnum() <= C.cutoff) {
	if (k == 0) { r = Root->isand() ? (logop*) new andop : (logop*) new orop; r->simpleflag = -1; }
	r->A.push_back(simplifiedPieces[j]);
	++k; 
	++j; }
      else {
	p->A.push_back(r);
	r = 0; k = 0; }
    }
    if (k != 0) {
      if (r->A.size() == 1) { p->A.push_back(r->A[0]); r->A[0] = 0; delete r; }
      else { p->A.push_back(r); }
    }
    p->simpleflag = 0; // Mark as simplify pieces
    q = simplify(p,C);
    delete p; 
  }

  return q;
}
