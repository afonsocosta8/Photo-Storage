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
#include <pthread.h>

#define CHUNK_SIZE 512

typedef struct _args{

  int mp;
  int p;
  char *h;

}args;

typedef struct _header{
  long	data_length;
} header;

int reg(char *host, int p){
  struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char buff[]="GET PEER";
	int nbytes;

	int sock_fd= socket(AF_INET, SOCK_DGRAM, 0);

	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	printf(" socket to gateway created \n Ready to send GET message\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(p);
	inet_aton(host, &server_addr.sin_addr);

	nbytes = sendto(sock_fd,
	                    buff, strlen(buff)+1, 0,
	                    (const struct sockaddr *) &server_addr, sizeof(server_addr));
	printf("sent %d %s\n", nbytes, buff);

  if (nbytes==-1) {
    printf("an error ocurred sending\n");
    exit(-1);
  }

  nbytes = recv(sock_fd, buff, 20, 0);
  printf("received %s\n", buff);

  if(strcmp(buff, "ERROR NO PEERS")){
    printf("no peer is available\n");
    exit(0);
  }

  // TCP connections to retrieve data_length


  strcpy(buff,"REG PEER");
  nbytes = sendto(sock_fd,
	                    buff, strlen(buff)+1, 0,
	                    (const struct sockaddr *) &server_addr, sizeof(server_addr));

  if (nbytes==-1) {
    printf("an error ocurred sending\n");
    exit(-1);
  }
	printf("sent %d %s\n", nbytes, buff);

  nbytes = recv(sock_fd, buff, 20, 0);

  printf("received %s\n", buff);

  return sock_fd;

}

void * handle_alive(void * arg){

  struct sockaddr_in local_addr;
	struct sockaddr_in client_addr;
	socklen_t size_addr;
	char buff[20];
	int nbytes;
  char *host;
  int p,mp;
  args *arguments = (args*)arg;
  host = arguments->h;
  p = arguments->p;
  mp = arguments->mp;

  int sock_fd = reg(host, p);


	local_addr.sin_family = AF_INET;
	local_addr.sin_port= (mp);
	local_addr.sin_addr.s_addr= INADDR_ANY;

	int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if(err == -1) {
		perror("bind");
		exit(-1);
	}

	printf(" socket created and binded \n Ready to receive message alive\n");

  size_addr = sizeof(client_addr);
  nbytes = recvfrom(sock_fd, buff, 20, 0,
          (struct sockaddr *) & client_addr, &size_addr);
  printf("received %d bytes from %s %d --- %s ---\n",
      nbytes, inet_ntoa(client_addr.sin_addr),client_addr.sin_port,  buff);
  if (strcmp(buff, "UALIVE")) {
    printf("MESSAGE UNEXPECTED\n");
    exit(-2);
  }

	sprintf(buff, "OK");
	nbytes = sendto(sock_fd,
	                    buff, strlen(buff)+1, 0,
	                    (const struct sockaddr *) &client_addr,
	                    sizeof(client_addr));
}


int main(int argc, char const *argv[]) {

  char *host, *port, *myport;
  port = calloc(128, sizeof(char));
  host = calloc(128, sizeof(char));
  in_port_t p, mp;
  pthread_t thr_id;
  args *arguments= (args*)malloc(sizeof(args));
  int psock;
  int i;
  if(argc!=7) { /* Number of arguments inserted is not correct */
      printf("Something went wrong...\nUsage: schat -n <name>.<survame> -i <ip> -p <scport> -s <snpip> -q <snpport>\n");
      exit(4);
  }else{ /*Reading each one of the arguments */
      for(i=1; i<argc; i=i+2) {
              switch(argv[i][1]) {

              case 'h':
                      strcpy(host, argv[i+1]);
                      break;

              case 'p':
                      strcpy(port, argv[i+1]);
                      break;

              case 'm':
                      strcpy(myport, argv[i+1]);
                      break;

              default:
                      printf("Something went wrong...\nUsage: schat -n <name>.<survame> -i <ip> -p <scport> -s <snpip> -q <snpport>\n");
                      exit(-1);
                      break;
              }
      }
  }
  p=atoi(port);
  mp=atoi(myport);
  arguments->mp=mp;
  arguments->h=host;
  arguments->p=p;

  if(pthread_create(&thr_id, NULL, handle_alive, arguments)!=0){
    printf("ERROR CREATING THREAD FOR READING ALIVES\n");
    exit(-1);
  }

  #ifdef DEBUG
    printf("\tDEBUG: THREAD CREATED\n");
  #endif

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
