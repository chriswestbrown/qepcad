#include "polynom.h"

fpart* parsefile(string fn);  // returns 0 if parse failed
fpart* parsestring(const char * const fn);// returns 0 if parse failed
char*  treetostring(fpart *root);
inline fpart* stringtotree(const char *p) { return parsestring(p); } // Just for Vespa
void   printversion(ostream&);


