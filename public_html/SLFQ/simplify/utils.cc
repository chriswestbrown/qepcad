#include "polynom.h"
#include "poly.tab.h"
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"

/***************************************************************
 *** Declarations for flex and bison functions and variables.
 ***************************************************************/
int yyparse (fpart* &result);
extern const char * stringin;
extern FILE* yyin;
extern int myflush();
extern "C" { int yywrap() {return 1; } };


/***************************************************************
 *** Function definitions
 ***************************************************************/
fpart* parsefile(string fn)
{
  // Open file
  FILE* input = (fn == "") ? stdin : fopen(fn.c_str(),"r"); 
  if (input == NULL) {
    cerr << "File " << fn << " not found!" << endl;
    exit(1); }

  // Parse file
  stringin = 0;
  yyin = input;
  fpart* p = 0;
  if (yyparse(p)) { p = 0; }
  myflush();
  if (input != stdin) fclose(input);
  return p;
}

fpart* parsestring(const char * const fn)
{
  yyin = (FILE*)0;
  stringin = fn;
  fpart* p = 0;
  if (yyparse(p)) { p = 0; }
  stringin = 0;
  return p;
}

char*  treetostring(fpart *root)
{
  ostringstream os;
  root->write(os);
  char *p = new char[os.str().length() + 1];
  os.str().copy(p,string::npos);
  p[os.str().length()] = 0;
  return p; // Whoever gets this array must delete it!
}

void printversion(ostream &OUT)
{
  /******* VERSION ********************************************/
  /* $Format: "static const char* version = \"$ProjectVersion$\";"$ */
static const char* version = "1.20";
  /* $Format: "static const char* versdate = \"$ProjectDate$\";"$ */
static const char* versdate = "Tue, 17 Apr 2012 14:04:33 -0400";


  OUT << "slfq version " << version << ' ' << versdate << endl;
}
