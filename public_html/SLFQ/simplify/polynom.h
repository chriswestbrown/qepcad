/***************************************************************
 ** This file defines the parse-tree data structure used by SLFQ.
 ** The base type for the tree is "fpart", and it just goes on
 ** from there!
 ***************************************************************/
#ifndef _SLFQ_POLYNOM_
#define _SLFQ_POLYNOM_
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <set>

using namespace std;

/* Strict Relational Operators */
#define LTOP   1
#define EQOP   2
#define GTOP   4

/* Other Relational Operators */
#define GEOP   6
#define NEOP   5
#define LEOP   3

/*******************************************************************
 *** BASE CLASS fpart formula part
 *******************************************************************/
class fpart
{
public:
  int simpleflag; //  0: formula never simplified (try simplifying pieces)
                  //  1: formula already simplified via QEPCAD (or constant!)
                  // -1: formula should be sent directly to QEPCAD to simplify
  virtual void write(ostream &out) = 0;
  virtual int isand() { return 0; }
  virtual int isor()  { return 0; }
  virtual int atomnum() { return -1; }
  virtual int depth() { return -1; }
  virtual int does_var_appear(const string &s) = 0;
  virtual int is_T_F() { return 0; }
  virtual void report(ostream &out) { out << "Not a formula!"; }
  virtual void varset(set<string> &S) = 0;
  set<string> varset() { set<string> S; varset(S); return S; }
  virtual fpart* copy() = 0;
  virtual ~fpart() { }
  virtual int max_degree(const string &v) {return 0; }
  virtual int max_tdegree(const vector<string> &VV, int i) {return 0; }  
  virtual int num_tcvar(const string &v) { return 0; }
};

/*******************************************************************
 *** BASE CLASS fpart formula part
 *******************************************************************/
class logconst : public fpart
{
  bool val;
 public:
  logconst(bool a, int b = 1) { val = a; simpleflag = b; }
  int is_T_F() { return 1; }
  bool value() { return val; }
  void write(ostream &out) { if (val) out << "TRUE"; else out << "FALSE"; }
  int atomnum() { return 0; }
  virtual int depth() { return 0; }
  int does_var_appear(const string &s) { return 0; }
  void report(ostream &out) { write(cout); }
  virtual logconst* copy() { return new logconst(val); }
  void varset(set<string> &S) { }
};

/*******************************************************************
 *** BASE CLASS ppart : polynomial part
 *******************************************************************/
class ppart : public fpart
{
public:
  virtual int isvar() { return  0; }
  virtual int isbinop() { return  0; }
  virtual ppart* sendneg() = 0;
  virtual ppart* sendleadingneg() { return sendneg(); }
  virtual ppart* copy() = 0;
};

/*******************************************************************
 *** negation operator
 *******************************************************************/
class negop : public ppart
{
public:
  ppart* operand;
  negop(ppart* p) : operand(p) { }
  void write(ostream &out) { out << "-"; operand->write(out); }
  ppart* sendneg() { ppart* p = operand; operand = 0; delete this; return p; }
  int does_var_appear(const string &s) { return operand->does_var_appear(s); }
  virtual ~negop() { if (operand) delete operand; }
  virtual negop* copy() { return new negop(operand->copy()); }
  void varset(set<string> &S) { operand->varset(S); }
  int max_degree(const string &v) { return operand->max_degree(v); }
  int max_tdegree(const vector<string> &VV, int i) {return operand->max_tdegree(VV,i); }  
  int num_tcvar(const string &v) { return operand->num_tcvar(v); }
};

/*******************************************************************
 *** var
 *******************************************************************/
class var : public ppart
{
public:
  string name;
  var(const char* s) : name(s) { }
  void write(ostream &out) { out << name; }
  int isvar() { return  1; }
  ppart* sendneg() { return new negop(this); }
  int does_var_appear(const string &s) { return (s == name); }
  virtual var* copy() { return new var(name.c_str()); }
  void varset(set<string> &S) { S.insert(name); }
  int max_degree(const string &v) { return (name == v) ? 1 : 0; }
  int max_tdegree(const vector<string> &VV, int i) { return max_degree(VV[i]); }
  int num_tcvar(const string &v) { return (name == v) ? 1 : 0; }
};

/*******************************************************************
 *** number
 *******************************************************************/
class number : public ppart
{
public:
  int sign;
  string value;
  number(const char* S, int a = 1): value(S), sign(a) { }
  void write(ostream &out) { if (sign == -1) out << '-'; out << value; }
  ppart* sendneg() { sign = -sign; return this; }
  int does_var_appear(const string &s) { return 0; }
  virtual number* copy() { return new number(value.c_str(),sign); }
  void varset(set<string> &S) {}
  int max_degree(const string &v) { return 0; }
  int int_value() const { int i = atoi(value.c_str()); return i*sign; }
  int max_tdegree(const vector<string> &VV, int i) {return 0; } 
};


/*******************************************************************
 *** binop
 *******************************************************************/
class binop : public ppart
{
public:
  ppart *left, *right;
  binop(ppart *p1 = NULL, ppart *p2 = NULL) : left(p1), right(p2) { }
  void write(ostream &out) { left->write(out); opwrite(out); right -> write(out);}
  int isbinop() { return  1; }
  virtual void opwrite(ostream &out) { out << " ? "; }
  int does_var_appear(const string &s) 
  { return left->does_var_appear(s) || right->does_var_appear(s); }
  virtual ~binop() { if (left) delete left; if (right) delete right; }
  void varset(set<string> &S) { left->varset(S); right->varset(S); }
  int max_degree(const string &v) { return max(left->max_degree(v),right->max_degree(v)); }
  int max_tdegree(const vector<string> &VV, int i) {
    return max(left->max_tdegree(VV,i),right->max_tdegree(VV,i)); 
  }
  int num_tcvar(const string &v) { return left->num_tcvar(v) + right->num_tcvar(v); }
};

/*******************************************************************
 *** minus, plus, product, exponentiation operators
 *******************************************************************/
class minusop : public binop
{
public:
  void opwrite(ostream &out) { out << " - "; }
  minusop(ppart *p1 = NULL, ppart *p2 = NULL) : binop(p1,p2) { }
  virtual ppart* sendneg() { swap(left,right); return this; }
  virtual minusop* copy() { return new minusop(left->copy(),right->copy()); }
  virtual ppart* sendleadingneg() { left = left->sendleadingneg(); return this; }
};

class addop : public binop
{
public:
  void opwrite(ostream &out) { out << " + "; }
  addop(ppart *p1 = NULL, ppart *p2 = NULL) : binop(p1,p2) { }
  virtual ppart* sendneg() 
  { 
    binop *p; 
    p = new minusop(left->sendneg(),right); 
    left = right = 0; 
    delete this; 
    return p; 
  }
  virtual addop* copy() { return new addop(left->copy(),right->copy()); }
  virtual ppart* sendleadingneg() { left = left->sendleadingneg(); return this; }
};

class prodop : public binop
{
public:
  void opwrite(ostream &out) { out << " "; }
  prodop(ppart *p1 = NULL, ppart *p2 = NULL) : binop(p1,p2) { }
  virtual ppart* sendneg() { left = left->sendneg(); return this; }
  virtual prodop* copy() { return new prodop(left->copy(),right->copy()); }
  int max_degree(const string &v) { return left->max_degree(v) + right->max_degree(v); }

  int max_tdegree(const vector<string> &VV, int i) {
    if (max_degree(VV[i]) == 0)
      return 0;
    else {
      int t = 0;
      for(int j = 0; j < VV.size(); ++j)
	t += max_degree(VV[j]);
      return t; }
  }
  int num_tcvar(const string &v) { return does_var_appear(v)? 1 : 0; }
};

class expop : public binop
{
public:
  void opwrite(ostream &out) { out << "^"; }
  expop(ppart *p1 = NULL, ppart *p2 = NULL) : binop(p1,p2) { }
  virtual ppart* sendneg() { return new negop(this); }
  virtual expop* copy() { return new expop(left->copy(),right->copy()); }
  int max_degree(const string &v) { return left->max_degree(v) * 
				      dynamic_cast<number*>(right)->int_value(); }
  int max_tdegree(const vector<string> &VV, int i) {
    if (left->max_degree(VV[i]) == 0)
      return 0;
    else {
      int n = dynamic_cast<number*>(right)->int_value();
      int t = 0;
      for(int j = 0; j < VV.size(); ++j)
	t += left->max_degree(VV[j]);
      return n*t; }
  }
  int num_tcvar(const string &v) { return does_var_appear(v)? 1 : 0; }
};


/*******************************************************************
 *** relop a relational operator -- no longer a "polynomial part"
 *** consists of a polynomial part and a relational operator code
 *******************************************************************/
class relop : public fpart
{
public:
  ppart *pol;
  int relopcode;
  relop(int a, ppart *p1, int b) : relopcode(a), pol(p1) { simpleflag = b; }
  relop(char* theop, ppart *p1 = NULL) : pol(p1) 
  { 
    simpleflag = -1;
    if (!strcmp(theop,"="))
      relopcode = EQOP;
    else if (!strcmp(theop,"<"))
      relopcode = LTOP;
    else if (!strcmp(theop,">"))
      relopcode = GTOP;
    else if (!strcmp(theop,"<="))
      relopcode = LEOP;
    else if (!strcmp(theop,">="))
      relopcode = GEOP;
    else if (!strcmp(theop,"<>"))
      relopcode = NEOP;
    else if (!strcmp(theop,"/="))
      relopcode = NEOP;
    else 
      relopcode = 0;
  }
  void varset(set<string> &S) { pol->varset(S); }
  void write(ostream &out) { pol->write(out); relwrite(out); out << 0; }
  void relwrite(ostream &out)
  {  
    if (relopcode == 0) out << " ? ";
    else if (relopcode == LTOP) out << " < ";
    else if (relopcode == EQOP) out << " = ";
    else if (relopcode == LEOP) out << " <= ";
    else if (relopcode == GTOP) out << " > ";
    else if (relopcode == NEOP) out << " /= ";
    else if (relopcode == GEOP) out << " >= ";
  }

  void setPoly(ppart *p1) { pol = p1; }

  int atomnum() { return 1; }
  int does_var_appear(const string &s) { return pol->does_var_appear(s); }
  virtual ~relop() { if (pol) delete pol; }
  virtual relop* copy() { return new relop(relopcode,pol->copy(),simpleflag); }
  void report(ostream &out) { out << "An atomic Tarski formula!"; }
  int max_degree(const string &v) { return pol->max_degree(v); }
  int max_tdegree(const vector<string> &VV, int i) {return pol->max_tdegree(VV,i); } 
  int num_tcvar(const string &v) { return pol->num_tcvar(v); }
};



/*******************************************************************
 *** irootexp consists of a variable, a relational operator, an 
 *** index and a polynomial.
 *******************************************************************/
class irootexp : public fpart
{
public:
  ppart *pol, *LHS;
  int relopcode;
  number *index;
  irootexp(int a, number* i, ppart *p0, ppart *p1, int b) : relopcode(a), index(i), LHS(p0), pol(p1) { simpleflag = b; }
  irootexp(number* i, ppart *p1 = NULL) : pol(p1), index(i)
  { 
    simpleflag = -1;
  }
  void varset(set<string> &S) { pol->varset(S); LHS->varset(S); }
  irootexp* setPoly(ppart *p, int op) { LHS = p; relopcode = op; return this; }
  void write(ostream &out) 
  { 
    LHS->write(out); out << ' '; 
    relwrite(out); 
    out << ' ' << "_root_";
    index->write(out);
    out << " "; 
    pol->write(out);
  }
  void relwrite(ostream &out)
  {  
    if (relopcode == 0) out << " ? ";
    else if (relopcode == LTOP) out << " < ";
    else if (relopcode == EQOP) out << " = ";
    else if (relopcode == LEOP) out << " <= ";
    else if (relopcode == GTOP) out << " > ";
    else if (relopcode == NEOP) out << " /= ";
    else if (relopcode == GEOP) out << " >= ";
  }

  int atomnum() { return 1; }
  int does_var_appear(const string &s) { return pol->does_var_appear(s) || LHS->does_var_appear(s); }
  virtual ~irootexp() { if (pol) delete pol; if (LHS) delete LHS; if (index) delete index; }
  virtual irootexp* copy() { return new irootexp(relopcode,index->copy(),LHS->copy(),pol->copy(),simpleflag); }
  void report(ostream &out) { out << "An atomic Extended Tarski formula!"; }
  int max_degree(const string &v) { return pol->max_degree(v); }
  int max_tdegree(const vector<string> &VV, int i) {return pol->max_tdegree(VV,i); } 
  int num_tcvar(const string &v) { return pol->num_tcvar(v); }
};

/*******************************************************************
 *** logop : logical operator
 *** This is a base class for logical operators.
     A : vector<fpart*> The operands
 *******************************************************************/
class logop : public fpart
{
public:
  vector<fpart*> A;
  logop() { simpleflag = 0; }
  logop(fpart *p1, fpart *p2) : A(2)
  {
    A[0] = p1;
    A[1] = p2;
    simpleflag = 0;
  }
  virtual ~logop() { for(int i = 0; i < A.size(); i++) if (A[i]) delete A[i]; }
  void varset(set<string> &S)
  {
    for(int i = 0; i < A.size(); i++)
      A[i]->varset(S);
  }
  int atomnum()
  {
    int sum = 0;
    for(int i = 0; i < A.size(); i++)
      sum += A[i]->atomnum();
    return sum;
  }
  virtual int depth()
  {
    int d = 0;
    for(int i = 0; i < A.size(); i++)
      d = max(A[i]->depth(),d);
    return d + 1;
  }
  int does_var_appear(const string &s) 
  {
    for(int i = A.size() - 1; i >= 0; i--)
      if (A[i]->does_var_appear(s))
	return 1;
    return 0;
  }
  virtual logop* copy() = 0;
  int max_degree(const string &v) 
  { 
    int m = 0;
    for(int i = 0; i < A.size(); ++i)
      m = max(m,A[i]->max_degree(v));
    return m;
  }
  int max_tdegree(const vector<string> &VV, int i) 
  { 
    int m = 0;
    for(int j = 0; j < A.size(); ++j)
      m = max(m,A[j]->max_tdegree(VV,i));
    return m;
  }

  int num_tcvar(const string &v) 
  {
    int m = 0;
    for(int j = 0; j < A.size(); ++j)
      m += A[j]->num_tcvar(v);
    return m;
  }

};



/*******************************************************************
 *** andop
 *******************************************************************/
class andop : public logop
{
public:
  andop() { }
  andop(fpart *p1, fpart *p2) : logop(p1,p2) { }
  int isand() { return 1; }

  static fpart* makeand(fpart *p1, fpart *p2)
  {
    if (p1->isand() && p2->isand()) {
      andop *q1 = (andop*)p1;
      andop *q2 = (andop*)p2;
      for(int i = 0; i < q2->A.size(); i++) {
	q1->A.push_back(q2->A[i]);
	q2->A[i] = NULL;
      }
      delete p2;
      return p1; }
    if (p1->isand()) {
      andop *q1 = (andop*)p1;      
      q1->A.push_back(p2);
      return p1; }
    if (p2->isand()) {
      cerr << "order swap!" << endl;
      andop *q2 = (andop*)p2;
      q2->A.push_back(p1);
      return p2; }
    return new andop(p1,p2); 
  }

  void write(ostream &out)
  {
    out << "[ ";
    A[0]->write(out);
    for(int i = 1; i < A.size(); i++) {
      out << " /\\ ";
      A[i]->write(out); }
    out << " ]";
  }   

  void report(ostream &out) 
  { out << "The AND of " << A.size() << " things!"; }

  virtual andop* copy() 
  { 
    andop *p = new andop(A[0]->copy(),A[1]->copy());
    for(int i = 2; i < A.size(); i++)
      p->A.push_back(A[i]->copy());
    p->simpleflag = simpleflag;
    return p;
  }
};

/*******************************************************************
 *** orop
 *******************************************************************/
class orop : public logop
{
public:
  orop() { }
  orop(fpart *p1, fpart *p2) : logop(p1,p2) { }
  int isor() { return 1; }
  static fpart* makeor(fpart *p1, fpart *p2)
  {
    if (p1->isor() && p2->isor()) {
      orop *q1 = (orop*)p1;
      orop *q2 = (orop*)p2;
      for(int i = 0; i < q2->A.size(); i++) {
	q1->A.push_back(q2->A[i]);
	q2->A[i] = NULL;
      }
      delete p2;
      return p1; }
    if (p1->isor()) {
      orop *q1 = (orop*)p1;      
      q1->A.push_back(p2);
      return p1; }
    if (p2->isor()) {
      cerr << "order swap!" << endl;
      orop *q2 = (orop*)p2;
      q2->A.push_back(p1);
      return p2; }
    return new orop(p1,p2); 
  }
  void write(ostream &out)
  {
    out << "[ ";
    A[0]->write(out);
    for(int i = 1; i < A.size(); i++) {
      out << " \\/ ";
      A[i]->write(out); }
    out << " ]";
  }   

  void report(ostream &out) 
  { out << "The OR of " << A.size() << " things!"; }

  virtual orop* copy() 
  { 
    orop *p = new orop(A[0]->copy(),A[1]->copy());
    for(int i = 2; i < A.size(); i++)
      p->A.push_back(A[i]->copy());
    p->simpleflag = simpleflag;
    return p;
  }
};

#endif
