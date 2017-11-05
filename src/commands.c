#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "commands.h"
#include "built_in.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>

static struct built_in_command built_in_commands[] = {
  { "cd", do_cd, validate_cd_argv },
  { "pwd", do_pwd, validate_pwd_argv },
  { "fg", do_fg, validate_fg_argv }
};

static int is_built_in_command(const char* command_name)
{
  static const int n_built_in_commands = sizeof(built_in_commands) / sizeof(built_in_commands[0]);

  for (int i = 0; i < n_built_in_commands; ++i) {
    if (strcmp(command_name, built_in_commands[i].command_name) == 0) {
      return i;
    }
  }

  return -1; // Not found
}



/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{ pidnumber = 0;
  int pid,status;//pid
  char a[1];
  int pidstore;
  fflush(stdin);
  fflush(stdout);  
  if (n_commands > 0){
     
   if(n_commands == 2)
  {
    struct single_command* com = (*commands);
     
    int sockets[2], rc;
    char buf[256];
    memset(buf,0,sizeof(buf));
    pid_t pid; 
    rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);
    if(rc== -1)
     {printf("error opening socket pair\n");
      exit(1);
     }

    pid=fork();
    if(pid == -1)

     { printf("fork error\n");}
    else if(pid ==0)
    {
      dup2(sockets[0],1);
      close(sockets[0]);
      close(sockets[1]);
      if(execv(com->argv[0], com->argv)==-1)
        { printf("error execution\n"); exit(1);}
    }
    else
    { wait(&status);
      close(sockets[0]);
      dup2(sockets[1],0);   //
      close(sockets[1]);    //
      rc = read(sockets[1],buf,sizeof(buf));
      printf("buffer: %s\n",buf);

      
      com = (*commands)+1;
  //   printf("\n%s\n",com->argv[0]);
  //   (com->argv)[com->argc]=(char*)malloc(sizeof(buf));
  //   strcpy((com->argv)[com->argc],buf);
       close(sockets[1]);
      
      for(int i=0; i<com->argc; i++)
     {printf("%s\n",com->argv[i]);}
    
      if(execv(com->argv[0],com->argv)==-1)
       {printf("second error\n"); exit(1);}
      return 0;
       
    }
    
    return 0;
  }//

 
    struct single_command* com = (*commands);
    assert(com->argc != 0);
    strncpy(a,&com->argv[0][0],1); //copy first one
    
    //for process creation
    if(strcmp("/",a) == 0)
      {
       printf("process creation start\n");
       if((pid=fork()) == -1)
         printf("fork failed\n");
       else if(pid != 0)
        {printf("%d\n",pid);
         pid=wait(&status); }
       else{
        if(execv(com->argv[0] , com->argv)==-1) //
          {printf("error execution\n");  exit(1); }  }      
      return 0;
     }
  // for path
   if(strcmp(com->argv[0],"ls") == 0 || strcmp(com->argv[0],"cat") == 0 || strcmp(com->argv[0],"vim") ==0)
   {
    if((pid=fork()) == -1)
      printf("fork failed\n");
    else if(pid != 0 )
      pid=wait(&status);
    else{    
       if(strcmp(com->argv[0],"ls") == 0)
        {execv("/bin/ls",com->argv); } //ls
       else if(strcmp(com->argv[0],"cat")==0)
        {execv("/bin/cat",com->argv);}//cat
       else if(strcmp(com->argv[0],"vim")==0)
       {execv("/usr/bin/vim",com->argv);} //vim
        }
    return 0;
   }

    int built_in_pos = is_built_in_command(com->argv[0]);
    if (built_in_pos != -1) {
      if (built_in_commands[built_in_pos].command_validate(com->argc, com->argv)) {
        if (built_in_commands[built_in_pos].command_do(com->argc, com->argv) != 0) {
          fprintf(stderr, "%s: Error occurs\n", com->argv[0]);
        }
      } else {
        fprintf(stderr, "%s: Invalid arguments\n", com->argv[0]);
        return -1;
      }
    } else if (strcmp(com->argv[0], "") == 0) {
      return 0;
    } else if (strcmp(com->argv[0], "exit") == 0) {
      return 1;
    } else {
      fprintf(stderr, "%s: command not found\n", com->argv[0]);
      return -1;
    }
  }
  
  return 0;
}

void free_commands(int n_commands, struct single_command (*commands)[512])
{
  for (int i = 0; i < n_commands; ++i) {
    struct single_command *com = (*commands) + i;
    int argc = com->argc;
    char** argv = com->argv;

    for (int j = 0; j < argc; ++j) {
      free(argv[j]);
    }

    free(argv);
  }

  memset((*commands), 0, sizeof(struct single_command) * n_commands);
}
