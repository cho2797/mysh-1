#ifndef MYSH_COMMANDS_H_
#define MYSH_COMMANDS_H_

struct single_command
{
  int argc;
  char** argv;
};

int evaluate_command(int n_commands, struct single_command (*commands)[512]);

void free_commands(int n_commands, struct single_command (*commands)[512]);

struct bg
{
int pidnumber; //pid number for backgorund process
char instruction[256];//instruction for background process
int flag;//for know another process alive 
};

struct bg background [1];
#endif // MYSH_COMMANDS_H_
