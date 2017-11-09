#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "commands.h"
#include "built_in.h"
#include "utils.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>


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
void sigchld(int signo)
{  
  int status, pid;
   if((pid = waitpid(-1,&status, 0)) != -1)
   {printf("%d done\n",pid);}

 }

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  
  int i=1; //
 
  int pid,status;//pid
  char b[1];
  int pidstore;
  fflush(stdin);
  fflush(stdout);  
  if (n_commands > 0){
  // for | implementation    
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
  //    close(sockets[1]);    //
  //    rc = read(sockets[1],buf,sizeof(buf));
  //    printf("buffer: %s\n",buf);

      
      com = (*commands)+1;
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
   strncpy(b,&com->argv[0][0],1); //copy first one
  
   
   if((strcmp("/",b) == 0) && (strcmp("&",com->argv[com->argc-1]) != 0 ) )
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
  

  
   //forg bg
    if(strcmp("&",com->argv[(com->argc-1)])==0)
   { 
     char buf[256];
     memset(buf,0,sizeof(buf));
     com->argv[com->argc-1]=NULL;
     com->argc = com->argc -1;
     signal(SIGCHLD,sigchld);
    
     if((pid=fork()) == -1)
       printf("fork failed\n");
     else if(pid != 0)
      { 
       while(1){
       fgets(buf,8096,stdin);
       printf("buf: %s\n",buf);
       
       struct single_command commands2[512];
       int n_commands2 = 0;
       mysh_parse_command(buf, &n_commands2, &commands2);
       //int built_in_pos2 = is_built_in_command(commands2->argv[0]);
       printf("pass\n");
       if(commands2->argc !=0)
       {if(strcmp(commands2->argv[0],"cd") == 0 )
       {printf("before cd\n");  do_cd(commands2->argc,commands2->argv);
        printf("after cd\n");}
       else if(strcmp(commands2->argv[0],"pwd")==0)
        do_pwd(commands2->argc,commands2->argv);
       else if(strcmp(commands2->argv[0],"fg")==0)
         do_fg(commands2->argc,commands2->argv);
       else 
        { printf("here\n");
         if(execv(commands2->argv[0], commands2->argv) == -1)
          {printf("error execution\n"); }
        }}

        free_commands(n_commands2, &commands2);
        n_commands2 = 0;
        }
        //pa
      }
     else
      { 
        printf("%d\n",getpid());
        if(execv(com->argv[0],com->argv)== -1)
          {printf("error execution\n"); exit(1);}

      }
   if((pid=waitpid(-1,&status,0)) != -1)
     printf("2  2  %d done\n",pid);
   
   printf("end of fg\n");
 //   free_commands(n_commands2, &commands2);
 //   n_commands2 = 0;
 //   free(buf);
    return 0;
  }

  // for path
  if(strcmp(com->argv[0],"ls") == 0 || strcmp(com->argv[0],"cat") == 0 || strcmp(com->argv[0],"vim") ==0)
   {


    char *path;
    path = getenv("PATH");
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

  //original code
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





