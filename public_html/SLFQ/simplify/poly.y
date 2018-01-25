%{
class fpart;
extern int lineno;
#include "polynom.h"
 int yylex();
 int yyerror(fpart* &result, const char* p)
  {
    cerr << lineno << ":gotta: " << p << " Parser's dyin'!" << endl;
    return 1;
  }
 extern "C" {
  extern int yywrap();
 };
%}

%parse-param { fpart* &result }

%union {
  fpart* ntType;
  fpart* tkType;
}

%type <ntType> formula atom aobj term mpolynomial polynomial rootindex
%token<tkType> VAR PLUS MINUS PROD EXP DIV NUMBER RELOPTOK ZERO ANDTOK ORTOK LEFTP RIGHTP RSYMBP RSYMBN TRUETOK FALSETOK
%left ORTOK
%left ANDTOK
%nonassoc IROOT
%left RELOPTOK
%nonassoc UMINUS
%left PLUS MINUS
%left PROD DIV
%left EXP

%%

twsm :       formula { result = $1; YYACCEPT; }
;

formula :    atom { $$ = $1; }
|            formula ANDTOK formula { $$ =  andop::makeand((fpart*)$1,(fpart*)$3); }
|            formula ORTOK  formula { $$ =  orop::makeor((fpart*)$1,(fpart*)$3); }
|            LEFTP formula RIGHTP { $$ = $2; }
;

atom :       polynomial RELOPTOK ZERO { ((relop*)$2)->setPoly((ppart*)$1); $$ = $2; }
|            polynomial RELOPTOK rootindex { $$ = ((irootexp*)$3)->setPoly((ppart*)$1,((relop*)$2)->relopcode); }
|            TRUETOK  { $$ =  new logconst(true); }
|            FALSETOK { $$ =  new logconst(false); }
;


aobj:        VAR {$$ = $1; }
|            NUMBER {$$ = $1; }
|            aobj EXP NUMBER  { $$ = new expop((ppart*)$1,(ppart*)$3); }
; 

term:        aobj {$$ = $1; }
|            term PROD aobj {  $$ = new prodop((ppart*)$1,(ppart*)$3); }
|            term DIV NUMBER { number* p = (number*)$3; p->value = "1/" + p->value; // HACK!
                               $$ = new prodop((ppart*)$1,(ppart*)$3); }
|            term aobj   { $$ = new prodop((ppart*)$1,(ppart*)$2); }
;

mpolynomial:  term {$$ = $1; }
|            mpolynomial PLUS mpolynomial  { $$ = new addop((ppart*)$1,(ppart*)$3); } 
|            mpolynomial MINUS mpolynomial { $$ = new minusop((ppart*)$1,(ppart*)$3); } 
;

polynomial:  mpolynomial { $$ = $1; }
|            MINUS mpolynomial { $$ = (((ppart*)$2)->sendleadingneg()); }
;

rootindex :  RSYMBP NUMBER polynomial %prec IROOT { $$ = new irootexp((number*)$2,(ppart*)$3); }
|            RSYMBN NUMBER polynomial %prec IROOT { $$ = new irootexp((number*)(((number*)$2)->sendneg()),(ppart*)$3); }
;
