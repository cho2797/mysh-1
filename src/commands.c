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
#include <sys/un.h>
#define SOCK_PATH "tpf_unix_sock.server"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
#define DATA "HELLO\n"

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
    { background[0].pidnumber = pid;
      background[0].flag = 1;
      printf("%d done %s\n",background[0].pidnumber, background[0].instruction);
    }
}
/* client side */
int client_side(struct single_command *com){
 
 printf("com %s\n",com->argv[0]);

 int client_sock, rc, len;
 struct sockaddr_un server_sockaddr;
 struct sockaddr_un client_sockaddr;
 char buf[256];
 memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
 memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
// char *arg[]={"ls",(char *)0};
//creat unix domain stream socket
 printf("creating         client \n");
 client_sock = socket(AF_UNIX, SOCK_STREAM, 0);
 if(client_sock == -1)
 {
  printf("socket error\n");
  exit(1); }
 printf("socket id: %d        client\n",client_sock);

//set up the unix sockaddr structure
 client_sockaddr.sun_family = AF_UNIX;
 strcpy(client_sockaddr.sun_path, CLIENT_PATH);
 len = sizeof(client_sockaddr);


 unlink(CLIENT_PATH);
 rc = bind(client_sock, (struct sockaddr *) &client_sockaddr, len);
 if(rc == -1)
  { printf("bind error\n");
    close(client_sock);
    exit(1); }

//set up the unix sockaddr strucure for the server socket and connect to it
 printf("connect wait         client\n");
 server_sockaddr.sun_family = AF_UNIX;
 strcpy(server_sockaddr.sun_path, SERVER_PATH);

 rc= connect(client_sock, (struct sockaddr *) &server_sockaddr, len);
 if(rc == -1)
 { printf("connect error\n");
   close(client_sock);
   exit(1);
 }

dup2(client_sock,1);
close(client_sock);

//execv("/bin/ls",arg);
execv(com->argv[0],com->argv);

printf("before close socket  client \n");

//close the socket
 close(client_sock);
 return 0;
}

/*server side*/
int server_side(struct single_command *com){
 printf("server start         server\n");
 int server_sock, client_sock, len, rc;
 int bytes_rec=0;
 struct sockaddr_un server_sockaddr;
 struct sockaddr_un client_sockaddr;
 char buf[256];
 int backlog =10;
// char *arg[]={"/bin/grep","tpf",(char *)0};


 memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
 memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
 memset(buf, 0, 256);


//creat a unix domain stream socket //

 server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
 if(server_sock == -1){
  printf("socket error\n");
  exit(1); }


 server_sockaddr.sun_family = AF_UNIX;
 strcpy(server_sockaddr.sun_path, SOCK_PATH);
 len = sizeof(server_sockaddr);

 unlink(SOCK_PATH);//

 rc = bind(server_sock, (struct sockaddr *) &server_sockaddr, len);
 if(rc == -1){
   printf("bind error\n");
   close(server_sock);
   exit(1);
 }

 printf("waiting for listen               server\n");
//listen for any client sockets//

 rc= listen(server_sock, backlog);
 if(rc == -1){
  printf("listen error \n");
  close(server_sock);
  exit(1);
  }

//accept an incoming connection//
printf("accepting an incoming data              server\n");
 client_sock = accept(server_sock, (struct sockaddr *) &client_sockaddr, &len);
 if(client_sock == -1){
  printf("accept error\n");
  close(server_sock);
  close(client_sock);
  exit(1);
  }

printf("get a name of the connected socket           server\n");
//get a neme of the xonnexted socket
 len = sizeof(client_sockaddr);
 rc = getpeername(client_sock, (struct sockaddr *) &client_sockaddr, &len);
 if(rc == -1){
  printf("gerpeername error\n");
  close(server_sock);
  close(client_sock);
  exit(1);
  }
 else { printf("client socket filepath: %s\n",client_sockaddr.sun_path);
 }

dup2(client_sock,0);
// reand and print
 printf("read data                     server\n");

if(execv(com->argv[0],com->argv)==-1)
{  printf("error in 2nd\n");
   rc = recv(client_sock, buf, sizeof(buf), 0);
   printf("buf is: %s\n",buf);
   (com->argv)[com->argc] = (char*)malloc(sizeof(buf));   
   strcpy((com->argv)[com->argc],buf);
   if(execv(com->argv[0],com->argv) == -1)
    {printf("error really\n"); exit(1);}
}

 memset(buf,0,256);

 close(server_sock);
 close(client_sock);

 return 0;
}

/*
 * Description: Currently this function only handles single built_in commands. You should modify this structure to launch process and offer pipeline functionality.
 */
int evaluate_command(int n_commands, struct single_command (*commands)[512])
{
  
  int i=1; //for  
  int pid,status;//pid
  int pidstore;
  int flag=0;
  fflush(stdin);
  fflush(stdout); 
    
 
  
  if (n_commands > 0){
  struct single_command* com = (*commands); 
  assert(com->argc != 0);

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
    return 0;
    }//cd ,pwd, fg implementation
    else if (strcmp(com->argv[0], "") == 0) {
      return 0; //" "
    } else if (strcmp(com->argv[0], "exit") == 0) {
      return 1; //exit
    }/* else {
      fprintf(stderr, "%s: command not found\n", com->argv[0]);
      return -1; //error
      }*/
   
  //original finish

  if(n_commands==2) flag=1; //ipc
  else if(strcmp(com->argv[0],"ls")==0 || strcmp(com->argv[0],"cat")==0 || strcmp(com->argv[0],"vim")==0) flag=2;//path
  else if(strcmp("&",com->argv[(com->argc-1)])==0) flag =3; //bg
  else flag = 4; //process creation

  printf("flag is %d\n",flag);

  switch(flag){

  case 1:                      
  //ipc
  {
	int pid, status;
	printf("start\n");
        printf("argv[0] = %s\n",com->argv[0]);
	if((pid=fork())==-1)
	  printf("error\n");
	else if(pid != 0)
	 {
 	 waitpid(-1,&status,WNOHANG);
 	 com = (*commands)+1;
         
         server_side(com);
	
         printf("server end\n");
 	 wait(&status);
	}//parent
	else
	{

	client_side(com);
	printf("clinet end\n");

	}
	return 0;

   
  }break;
 
  case 2:
  //path     
  {
    if((pid=fork()) == -1)
       printf("fork failed\n");
    else if(pid != 0 )
       pid=wait(&status);
    else{
       if(strcmp(com->argv[0],"ls")==0)
        {if(execv("bin/ls",com->argv)==-1)
            printf("error execution of ls\n");}
       else if(strcmp(com->argv[0],"cat")==0)
        {if(execv("bin/cat",com->argv)==-1)
            printf("error execution of cat\n");}
      else if(strcmp(com->argv[0],"vim")==0)
        {if(execv("/usr/bin/ls",com->argv)==-1)
            printf("error execution of vim\n");} }
     return 0;
  }break;
  
  case 3:
  //bg
   {
     char buf[256];
     memset(buf,0,sizeof(buf));
     com->argv[com->argc-1]=NULL;
     com->argc = com->argc -1;
     signal(SIGCHLD,sigchld);
    
      memset(background[0].instruction,0,sizeof(background[0].instruction));
      background[0].pidnumber =0;  background[0].flag=0;
      
     if((pid=fork()) == -1)
       printf("fork failed\n");
     else if(pid != 0)
      { 
       
       strcpy(background[0].instruction,com->argv[0]);
       while(1){
       
       fgets(buf,8096,stdin);       
       struct single_command commands2[512];
       int n_commands2 = 0;
       mysh_parse_command(buf, &n_commands2, &commands2);
        
      
       if(commands2->argc !=0)
      {
        
       if(strcmp(commands2->argv[0],"cd") == 0 )
         do_cd(commands2->argc,commands2->argv); 
       else if(strcmp(commands2->argv[0],"pwd")==0)
        do_pwd(commands2->argc,commands2->argv);
       else if(strcmp(commands2->argv[0],"fg")==0)
         do_fg(commands2->argc,commands2->argv);
       else if(strcmp(commands2->argv[0],"exit")==0)
         exit(1);
       else 
        { 
         if(execv(commands2->argv[0], commands2->argv) == -1)
          {printf("error execution\n"); }
        }

       
      }
      /*
   
        int ret = evaluate_command(n_commands2, &commands2);
        
        free_commands(n_commands2, &commands2);
        n_commands2 = 0;
        if(ret == 1){ break;}  
        }
    */
      //wihle
    }}
    else
      {
        printf("%d\n",getpid());
        if(execv(com->argv[0],com->argv)== -1)
          {printf("error execution\n"); exit(1);}
      }

   return 0;
  }//bg 
  break;
  case 4:
  //process
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
  }break;
 }//switch 
  return 0;
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




