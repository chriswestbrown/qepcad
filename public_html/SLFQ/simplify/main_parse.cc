/*************************************************
 * This program provides a buttugly way to parse
 * quantifier-free formulas from Redlog or Qepcad
 * syntax.
 *************************************************/
#include "polynom.h"
#include <stdio.h>
#include "poly.tab.h"

/******************** Globals from lex & yacc ***********/
extern FILE* yyin;
extern void myflush();
extern "C" {
  int yywrap() {return 1; }
};
/********************************************************/


/********************************************************
 * This simply uses the lex/yacc modules to parse input.
 ********************************************************/
fpart* parse_form(FILE* input)
{
  yyin = input;
  fpart* p = 0;
  if (yyparse(p)) { p = 0; }
  myflush();
  return p;
}



/********************************************************
 * main function
 ********************************************************/
int main(int argc, char **argv)
{
  //-- Check argument number. --------------------------//
  if (argc < 2) {
    cerr << "require file to process as 1st argument!" << endl;
    exit(1); }

  //-- Open file & check existence. --------------------//
  FILE* input = fopen(argv[1],"r"); 
  if (input == NULL) {
    cerr << "File not found!" << endl;
    exit(1); }

  //-- Parse file and close input stream. --------------//
  fpart* p = parse_form(input);
  fclose(input);
  
  //-- Report findings. --------------------------------//
  {
    cout << "Here's what you've got: ";
    p->report(cout);
    cout << endl;
    
    logop* q = dynamic_cast<logop*>(p);
    if  (q) {
      do {
	cout << "Enter an index to check out that sub-formula, zero to quit: ";
	int i;
	if (!(cin >> i) || i <= 0 || i > q->A.size()) break;
	q->A[i-1]->write(cout);
	cout << endl;
      } while(1); }
  }
  
  //-- Write output in QEPCAD syntax. ------------------//
  char S[50];
  cout << "Enter output file name: (X for stdout) ";
  cin >> S;
  cerr << S << endl;
  if (S[0] != 'X') {
    ofstream out(S);
    p->write(out);}
  else {
    p->write(cout); }
  cout << endl;

  //----------------------------------------------------//
  return 0;
}
