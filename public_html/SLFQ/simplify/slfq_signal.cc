#include <sstream>
#include <signal.h>
#include <cstdlib>
#include <ctime>
#include "slfq_signal.h"
#include "simplify.h"

int last_call_time = -1;

void SIGINT_handler(int i, siginfo_t *sip,void* uap)
{
  int t = clock();
  if (last_call_time != -1 && t - last_call_time < CLOCKS_PER_SEC)
  {
    QEPCAD_SIGINT_handler(i,sip,uap); // Call QEPCAD's handler
    exit(1);
  }
  last_call_time = t;

  if (curr_form)
    cerr << "QEPCADB is simplifying ...\n" << curr_form->str() << endl;
  else
    cerr << "SLFQ is not waiting on QEPCADB right now." << endl;

  return;
}

void init_SIGINT_handler() 
{
  struct sigaction *p;
  p = (struct sigaction *)malloc(sizeof(struct sigaction));
  p->sa_handler = NULL;
  p->sa_sigaction = SIGINT_handler;
  sigemptyset(&(p->sa_mask));
  p->sa_flags = SA_SIGINFO;
  sigaction(SIGINT,p,NULL);
  free(p);
}

