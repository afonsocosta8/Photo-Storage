#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void * handle_get(void * arg){

  struct sockaddr_in client_addr = *(struct sockaddr_in *)arg;
  int resp_fd = socket(AF_INET, SOCK_DGRAM, 0);
  char resp_buff[100];
  int nbytes;

  // GET PEER ADDRESS AND SPRINTF TO buff

  #ifdef DEBUG
    printf("\t\tDEBUG: SERVING CLIENT %s:%d WITH PEER %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, "192.168.2.2", 5000);
  #endif

  sprintf(resp_buff, "OK %s:%d", "192.168.2.2", 5000);
  nbytes = sendto(resp_fd, resp_buff, strlen(resp_buff)+1, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr));

  #ifdef DEBUG
    printf("\t\tDEBUG: SENT %dB TO CLIENT %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), client_addr.sin_port, resp_buff);
  #endif

  return;
}

void * handle_reg(void * arg){

  struct sockaddr_in client_addr = *(struct sockaddr_in *)arg;
  int resp_fd = socket(AF_INET, SOCK_DGRAM, 0);
  char resp_buff[100];

  // GET PEER ADDRESS AND SPRINTF TO buff

  #ifdef DEBUG
    printf("\t\tDEBUG: REGISTERING PEER OK\n");
  #endif
  sprintf(resp_buff, "OK");
  sendto(resp_fd, resp_buff, strlen(resp_buff)+1, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr));

  return;
}


int main(){


  // CONNTECTION RELATED
  struct sockaddr_in local_addr;
  struct sockaddr_in client_addr;
  socklen_t size_addr;
  int sock_fd;
  int nbytes;
  char buff[100];

  // THREAD RELATED
  pthread_t thr_id;

  // PEERS LIST RELATED

  #ifdef DEBUG
    printf("\tDEBUG: CREATING SOCKET...\n");
  #endif

  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock_fd == -1){
    perror("ERROR CREATING SOCKER\n");
    exit(-1);
  }

  #ifdef DEBUG
    printf("\tDEBUG: SOCKET No: %d\n\tDEBUG: BINDING...\n", sock_fd);
  #endif

  local_addr.sin_family = AF_INET;
	local_addr.sin_port= htons(9000);
	local_addr.sin_addr.s_addr= INADDR_ANY;
  if(bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
    perror("ERROR BINDIND");
    exit(-1);
  }

  printf("READY TO RECEIVE MESSAGES\n");

  while(1){

    size_addr = sizeof(client_addr);
    nbytes = recvfrom(sock_fd, buff, 100, 0, (struct sockaddr *) &client_addr, &size_addr);
    #ifdef DEBUG
      printf("\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), client_addr.sin_port,  buff);
    #endif

    printf("NEW CLIENT CONNECTED\n");

    if(strcmp(buff, "GET PEER")==0){
      #ifdef DEBUG
        printf("\tDEBUG: DECODED AS GET PEER\n\tDEBUG: CREATING THREAD FOR CLIENT...\n");
      #endif

      if(pthread_create(&thr_id, NULL, handle_get, &client_addr)!=0){
        printf("ERROR CREATING THREAD FOR CLIENT %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
        exit(-1);
      }

      #ifdef DEBUG
        printf("\tDEBUG: THREAD CREATED\n");
      #endif



    }else if(strcmp(buff, "REG PEER")==0){
      #ifdef DEBUG
        printf("\tDEBUG: DECODED AS REG PEER\n");
      #endif



    }else{
      #ifdef DEBUG
        printf("\tDEBUG: RECV INVALID COMMAND\n");
      #endif


    }


  }
  exit(0);




}
