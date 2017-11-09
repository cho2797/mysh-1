#include "signal_handlers.h"
#include <signal.h>
#include <stdio.h>



void catch_sigint(int signalNo)
{     
     signal(SIGINT,SIG_IGN);
  // TODO: File this!
}

void catch_sigtstp(int signalNo)
{
  printf("ctrl + z\n");
  signal(SIGTSTP,SIG_IGN);
  // TODO: File this!
}


