#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv)
{
  int accept_desc;
  int sockdesc;
  int fromlen;
  int port;
  struct hostent *hent;
  struct sockaddr_in sockadd;
  struct sockaddr_in fromadd;
  unsigned char* buffer;
  unsigned char* chr_ptr;

  if(argc > 1)
    port = atoi(argv[1]);
  else
    port = 9801;

  if((sockdesc = socket(PF_INET, SOCK_STREAM, 0)) < 0){
    perror("fail create socket\n");
    exit(1);
  }

  sockadd.sin_addr.s_addr = htonl(INADDR_ANY);
  sockadd.sin_family = AF_INET;
  sockadd.sin_port = htons(port);
  if(bind(sockdesc, &sockadd, sizeof(sockadd)) < 0){
    perror("fail bind\n");
    exit(2);
  }

  buffer = (unsigned char*)malloc(BUFSIZ * 10);

  listen(sockdesc, 1);
  while(1){
    accept_desc = accept(sockdesc, &fromadd, &fromlen);
    while(1){
      do{
	fromlen = read(accept_desc,buffer,BUFSIZ * 10);
      } while(fromlen < 1);
      buffer[fromlen] = '\0';
      while((chr_ptr = strstr(buffer,"\r")) != NULL){
	strcpy(chr_ptr,chr_ptr + 1);
      }
      if(strstr(buffer,"Script:")){
	printf("%s\n",strstr(buffer,"Script:") + strlen("Script:"));
	fflush(stdout);
      }
	
      if(strncmp(buffer,"\n",2) == 0 || strstr(buffer,"\n\n")) break;
    }
    write(accept_desc,"200 OK\r\n",strlen("200 OK\r\n"));
    close(accept_desc);
  }
  free(buffer);
}
