#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#define SOCK_PATH "tpf_unix_sock.server"
#define SERVER_PATH "tpf_unix_sock.server"
#define CLIENT_PATH "tpf_unix_sock.client"
#define DATA "hallo from client"

int client_side(void);
int server_side(void);

int main(void)
{
int pid, status;
printf("start\n");

if((pid=fork())==-1)
 printf("error\n");
else if(pid != 0)
 {
  waitpid(-1,&status,WNOHANG);
  //  wait(&status);
  server_side();
  printf("server end\n");
  wait(&status);
}//parent
else
{

client_side();
printf("clinet end\n");

}

return 0;
}


int client_side(void){
 printf("client begin               client\n");
 int client_sock, rc, len;
 struct sockaddr_un server_sockaddr;
 struct sockaddr_un client_sockaddr;
 char buf[256];
 memset(&server_sockaddr, 0, sizeof(struct sockaddr_un));
 memset(&client_sockaddr, 0, sizeof(struct sockaddr_un));
 char *arg[]={"ls",(char *)0};
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

execv("/bin/ls",arg);

printf("before close socket  client \n");

//close the socket
 close(client_sock);
 return 0;
}


int server_side(void){
 printf("server start         server\n");
 int server_sock, client_sock, len, rc;
 int bytes_rec=0;
 struct sockaddr_un server_sockaddr;
 struct sockaddr_un client_sockaddr;
 char buf[256];
 int backlog =10;
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
//accept an incoming connextion//

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

// reand and print
 printf("read data                     server\n");
 bytes_rec = recv(client_sock, buf, sizeof(buf), 0);
 if(bytes_rec == -1)
  {printf("error\n");
   close(server_sock);
   close(client_sock);
   exit(1);
  }
  else{
  printf("server  %s\n",buf);}

 memset(buf,0,256);

 close(server_sock);
 close(client_sock);

 return 0;
}


