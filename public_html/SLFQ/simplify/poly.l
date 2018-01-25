%{
#include "polynom.h"
#include "poly.tab.h"
#include <stdlib.h>

/***
To have lex read from a file, you need to set "yyin" to
a FILE* for the file you want - make sure "stringin" is
zero!.  To have lex read from a string, you need set 
"yyin" to 0, and set "stringin" to point to the string
you want to parse.  NOTE: "string" is char*, i.e. C-style.
***/

/*** G L O B A L    V A R I A B L E S *******************/
const char * stringin = 0;
static int stupidflag = 0;
static int rootflag = 0;
int lineno = 0;

/*** F U N C T I O N   D E F I N I T I O N S ***********/
int yyerror(fpart* &result, const char* p);

// This function is templated because of a mismatch in different
// bison versions.  Some call YY_INPUT with a second argument of
// type int, some with type yy_size_t.  By defining this via
// templates, either will compile!
template<class T>
inline void Func_YY_INPUT(char * const buf, T &result, yy_size_t max_size)
{
  fpart* dummy;
  if (stringin != 0) 
  { 
    size_t n; 
    for ( n = 0; n < max_size && *stringin != '\0' && *stringin != '\n'; n++) 
      buf[n] = *(stringin++); 
    if (n < max_size && *stringin == '\n') 
      buf[n++] = *(stringin++); 
    else if (n < max_size && *stringin == EOF && ferror( yyin ) ) 
      yyerror(dummy, "input in flex scanner failed" ); 
    result = n; 
  } 
//  else if ( yy_current_buffer->yy_is_interactive ) 
  else if ( YY_CURRENT_BUFFER->yy_is_interactive ) 
  { 
    int c = '*'; size_t n; 
    for ( n = 0; n < max_size && (c = getc( yyin )) != EOF && c != '\n'; ++n ) 
      buf[n] = (char) c;
    if ( c == '\n' )
      buf[n++] = (char) c;
    if ( c == EOF && ferror( yyin ) )
      yyerror(dummy, "input in flex scanner failed" );
    result = n;
  }
  else if ( ((result = fread( buf, 1, max_size, yyin )) == 0) 
	    && ferror( yyin ) ) 
    yyerror(dummy, "input in flex scanner failed" );
}

void myflush() { YY_FLUSH_BUFFER; }
#define YY_INPUT(buf,result,max_size) Func_YY_INPUT(buf,result,max_size);
%}

%%

[\t ]+ { };
"\n"   { lineno++; }

"+"  { return PLUS; }
"-"  { return MINUS; }
"*"  { return PROD; }
"**" |
"^"  { return EXP; }
"/"  { return DIV; }

"="  |
"<"  |
">"  |
"<=" |
">=" |
"<>" |
"/=" { yylval.ntType =  new relop(yytext); return RELOPTOK; }

"0"  { return ZERO; }
[1-9][0-9]* { rootflag = 0; yylval.ntType =  new number(yytext); return NUMBER; }

"TRUE" |
"true"  { return TRUETOK; }
"FALSE" |
"false" { return FALSETOK; }

"and" |
"/\\" { return ANDTOK; }
"or" |
"\\/"  { return ORTOK; }

"(" |
"["   { return LEFTP; }
")" |
"]"   { return RIGHTP; }

"_root_-" { rootflag = 1; return RSYMBN; }
"_root_"  { rootflag = 1; return RSYMBP; }

[a-zA-Z][a-zA-Z0-9_]* { yylval.ntType =  new var(yytext); return VAR; }

";"  { return ';'; }
