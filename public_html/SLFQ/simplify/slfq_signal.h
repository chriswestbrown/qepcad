#ifndef _SLFQ_SIGNAL_
#define _SLFQ_SIGNAL_

void init_SIGINT_handler();


//<<< Thise are copies of QEPCAD's handlers. At some point, I
// need to change qepcad so these are just linked from QEPCAD
#include "qepcad.h"
#include <iostream>
#include <fstream>
#include "db/convenientstreams.h"
#include <signal.h>
#include "db/CAPolicy.h"

static void QEPCAD_SIGINT_handler(int i, siginfo_t *sip,void* uap)
{
  // Kill child CAServer processes
  for(ServerBase::iterator p = GVSB.begin(); p != GVSB.end(); ++p)    
    p->second->kill();
  ENDSACLIB(SAC_FREEMEM);
  exit(1);
}

static void QEPCAD_init_SIGINT_handler() 
{
  struct sigaction *p;
  p = (struct sigaction *)malloc(sizeof(struct sigaction));
  p->sa_handler = NULL;
  p->sa_sigaction = QEPCAD_SIGINT_handler;
  sigemptyset(&(p->sa_mask));
  p->sa_flags = SA_SIGINFO;
  sigaction(SIGINT,p,NULL);
  sigaction(SIGTERM,p,NULL);
  free(p);
}
//>>>

#endif

