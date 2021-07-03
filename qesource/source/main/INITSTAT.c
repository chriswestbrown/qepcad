/*======================================================================
                      INITSTAT()

Initialize Statistics.
======================================================================*/
#include "qepcad.h"

void INITSTAT()
{
Step5: /* Statistics on Databases. */
       TMDBMNG = 0;
       TMDBSAV = 0;
       goto Return;

Return: /* Prepare for return. */
       return;
}
