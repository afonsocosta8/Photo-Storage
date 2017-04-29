#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#define CHUNK_SIZE 512

typedef struct _header{
  long	data_length;
} header;



int main(int argc, char const *argv[]) {
  
  struct sockaddr_in local_addr;
  struct sockaddr_in client_addr;
  socklen_t size_addr;
  header hdr;

  char buff[100];
  int nbytes;
  FILE *fp = fopen("/home/afonso/Documents/ist1.jpg","wb");
  int sock_fd= socket(AF_INET, SOCK_STREAM, 0);

  if (sock_fd == -1){
    perror("socket: ");
    exit(-1);
  }


  local_addr.sin_family = AF_INET;
  local_addr.sin_port= htons(9000);
  local_addr.sin_addr.s_addr= INADDR_ANY;
  int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
  if(err == -1) {
    perror("bind");
    exit(-1);
  }
  printf(" socket created and binded \n");

  listen(sock_fd, 5);


  while(1){
    printf("Ready to accept connections\n");
    int client_fd= accept(sock_fd, (struct sockaddr *) & client_addr, &size_addr);

    //if(fork()==0){
      printf("Accepted one connection from %s \n", inet_ntoa(client_addr.sin_addr));
      nbytes = recv(client_fd, buff, 100, 0);
      printf("received %d bytes --- %s ---\n", nbytes, buff);

      sprintf(buff, "OK");
      nbytes = send(client_fd, buff, strlen(buff)+1, 0);
      printf("replying %d bytes\n", nbytes);

      nbytes = recv(client_fd, (&hdr), sizeof(hdr), 0);
      printf("received %d bytes --- %ld ---\n", nbytes, hdr.data_length);
      unsigned char *buffer = malloc(hdr.data_length);

      nbytes = recv(client_fd, buffer, hdr.data_length, 0);

      sprintf(buff, "OK");
      nbytes = send(client_fd, buff, strlen(buff)+1, 0);
      printf("replying %d bytes\n", nbytes);

      fwrite(buffer,1,hdr.data_length,fp);

      close(client_fd);
      printf("closing connectin with client\n");
      exit(0);
    //}
  }
  close(sock_fd);
  exit(0);
  return 0;
}
