#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "commands.h"
#include "built_in.h"
#include "utils.h"
#include "signal_handlers.h"
#include <unistd.h>


int main()
{
  char buf[8096];

  while (1) {
   
    signal(SIGINT,catch_sigint);
    signal(SIGTSTP,catch_sigtstp);
    fgets(buf, 8096, stdin);
    struct single_command commands[512];
    int n_commands = 0;
    mysh_parse_command(buf, &n_commands, &commands);
    int ret = evaluate_command(n_commands, &commands);
    
    free_commands(n_commands, &commands);
    n_commands = 0;

    if (ret == 1) {
       break;
    }
  }

  return 0;
}
