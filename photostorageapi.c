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
#include <time.h>
#include "photostorageapi.h"


int gallery_connect(char * host, in_port_t p){

  struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char query_buff[]="GET PEER";
  char buff[100];
  char *port;
  char *ipport;
  char *ip;
  char gb[4];
	int nbytes;
  char *pt;
  int sock_fd;
  time_t start, end;



  // CREATING SOCKET FOR CONNECTIONS
  #ifdef DEBUG
    printf("\tDEBUG: CREATING SOCKET...\n");
  #endif

  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock_fd == -1){
	  perror("ERROR CREATING SOCKER\n");
		exit(-1);
	}

  #ifdef DEBUG
    printf("\tDEBUG: SOCKET No: %d\n", sock_fd);
  #endif



  // PREPATING TO SEND MESSAGE TO GATEWAY
  #ifdef DEBUG
    printf("\tDEBUG: PREPARING MESSAGE TO GATEWAY\n");
  #endif
  server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(p);
	inet_aton(host, &server_addr.sin_addr);



  // SENDING MESSAGE TO GATEWAY
  #ifdef DEBUG
    printf("\tDEBUG: SENDING MESSAGE TO GATEWAY...\n");
  #endif

  nbytes = sendto(sock_fd, query_buff, strlen(query_buff)+1, 0, (const struct sockaddr *) &server_addr, sizeof(server_addr));
  if(nbytes==-1){

    #ifdef DEBUG
      printf("\tDEBUG: GATEWAY UNAVAILABLE.");
    #endif

    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: SENT %dB TO GATEWAY %s:%d --- %s ---\n", nbytes, inet_ntoa(server_addr.sin_addr), server_addr.sin_port, query_buff);
  #endif



  // RECEIVE MESSAGE FROM GATEWAY
  nbytes = recv(sock_fd, buff, 20, 0);

  #ifdef DEBUG
    printf("\tDEBUG: ENDED WAITING FOR GATEWAY RESPONSE.\n");
  #endif

  if(nbytes <= 0){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SEND MESSAGE TO GATEWAY.\n");
    #endif
    return -1;
  }

  // MESSAGE RECEIVED
  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(server_addr.sin_addr), server_addr.sin_port,  buff);
  #endif

  // NO PEERS CASE
  if(strcmp(buff, "ERROR NO PEERS")==0){
    #ifdef DEBUG
      printf("\tDEBUG: NO PEERS AVAILABLE\n");
    #endif
    return 0;
  }

  // HANDLING IP AND PORT
  ipport = buff + 3;
  int i = 0;
  pt = strtok (ipport,":");
  while (pt != NULL) {
    if(i==0)
      ip=pt;
    else
      port = pt;
    pt = strtok (NULL, ":");
    i++;
  }
  #ifdef DEBUG
    printf("\tDEBUG: DECODED MESSAGE IP:PORT AS %s:%s\n", ip, port);
  #endif

  // CREATING TCP SOCKET TO CONNECT TO PEER
  sock_fd= socket(AF_INET, SOCK_STREAM, 0);

	if(sock_fd == -1){
		perror("ERROR CREATING SOCKET\n");
	  return -1;
	}

  #ifdef DEBUG
    printf("\tDEBUG: TCP SOCKET %d CREATED\n", sock_fd);
  #endif

	server_addr.sin_family = AF_INET;
	server_addr.sin_port= htons(atoi(port));
	inet_aton(ip, &server_addr.sin_addr);

	if(connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr)==-1)){
    #ifdef DEBUG
      printf("\tDEBUG: ERROR CONNECTING TO PEER %s:%s\n", ip, port);
    #endif
		return -1;
	}

  #ifdef DEBUG
    printf("\tDEBUG: RETURNING TCP SOCKET %d\n", sock_fd);
  #endif

  return sock_fd;
}




uint32_t gallery_add_photo(int peer_socket, char *file_name){

  int t, i;
  char buff[100];
  int nbytes;

  // OPENING FILE
  #ifdef DEBUG
    printf("\tDEBUG: OPENING FILE\n");
  #endif

  FILE *img = fopen(file_name, "rb");
  if(img == NULL)
    return 0;


  // GET FILE CHARECTERISTICS
  fseek(img, 0, SEEK_END);
  size_t filesize = ftell(img);
  fseek(img, 0, SEEK_SET);


  // PREPARING PROTOCOL MESSAGE TO PEER
  sprintf(buff, "ADD PHOTO %s SIZE %d", file_name, filesize);

  if(send(peer_socket, buff, sizeof(buff), 0)==-1)
    return 0;

  #ifdef DEBUG
    printf("\t\tDEBUG: SENT TO PEER --- %s ---\n", buff);
  #endif


  // STORE READ DARA INTO BUFFER
  unsigned char *buffer = malloc(filesize);
  fread(buffer, sizeof *buffer, filesize, img);

  nbytes = recv(peer_socket, buff, sizeof(buff), 0);
  if(nbytes == -1)
    return 0;

  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
  #endif

  if(strcmp(buff,"OK")!=0)
    return 0;

  if(send(peer_socket, buffer, filesize, 0)==-1)
    return 0;

  nbytes = recv(peer_socket, buff, sizeof(buff), 0);
  if(nbytes == -1)
    return 0;

  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
  #endif
  char answer[10], photoid[10];
  sscanf(buff, "%s %s", answer, photoid);

  #ifdef DEBUG
    printf("\tDEBUG: DECODED FILEID %s\n", buff);
  #endif

  return atoi(photoid);

}

int gallery_add_keyword(int peer_socket, uint32_t id_photo, char *keyword){

}

int gallery_search_photo(int peer_socket, char * keyword, uint32_t ** id_photo){

}

int gallery_delete_photo(int peer_socket, uint32_t id_p){

}

int gallery_get_photo_name(int peer_socket, uint32_t id_photo, char **photo_name){

}

int gallery_get_photo(int peer_socket, uint32_t id_photo, char *file_name){

}
