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
#include <sys/time.h>
#include "photostorageapi.h"

#define CHUNK_SIZE 512

int gallery_connect(char * host, in_port_t p){

  struct sockaddr_in server_addr;
	char query_buff[]="GET PEER";
  char buff[100];
  char *port;
  char *ipport;
  char *ip;
	int nbytes;
  char *pt;
  int sock_fd;

  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 500000;


  // CREATING SOCKET FOR CONNECTIONS
  #ifdef DEBUG
    printf("\tDEBUG: CREATING SOCKET...\n");
  #endif

  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock_fd == -1){
    perror("ERROR CREATING SOCKET\n");
    #ifdef DEBUG
      printf("\t\tDEBUG: COULD NOT CREATE SOCKET\n");
    #endif
    return -1;
  }

  /*if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0){
    perror("ERROR SETTING SOCKET OPTS\n");
    #ifdef DEBUG
      printf("\t\tDEBUG: COULD NOT SET SOCKET OPTS\n");
    #endif
    return -1;
  }*/

  #ifdef DEBUG
    printf("\tDEBUG: SOCKET No: %d\n", sock_fd);
  #endif



  // PREPARING TO SEND MESSAGE TO GATEWAY
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
      printf("\tDEBUG: GATEWAY UNAVAILABLE.\n");
    #endif
    close(sock_fd);
    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: SENT %dB TO GATEWAY %s:%d --- %s ---\n", nbytes, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), query_buff);
  #endif



  // RECEIVE MESSAGE FROM GATEWAY
  nbytes = 0;
  nbytes = recv(sock_fd, buff, 100, 0);
  if(nbytes <= 0){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT RECV MESSAGE FROM GATEWAY.\n");
    #endif
    close(sock_fd);
    return -1;
  }

  // MESSAGE RECEIVED
  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port),  buff);
  #endif

  // NO PEERS CASE
  if(strcmp(buff, "ERROR NO PEERS")==0){
    #ifdef DEBUG
      printf("\tDEBUG: NO PEERS AVAILABLE\n");
    #endif
    close(sock_fd);
    return 0;
  }
  if(strstr(buff,"OK")==NULL){
    close(sock_fd);
    return -1;
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
    #ifdef DEBUG
      printf("\t\tDEBUG: COULD NOT CREATE SOCKET\n");
    #endif
    return 0;
  }
  #ifdef DEBUG
    printf("\tDEBUG: TCP SOCKET %d CREATED\n", sock_fd);
  #endif



  // CONNECTING TO PEER
	server_addr.sin_family = AF_INET;
	server_addr.sin_port= htons(atoi(port));
	inet_aton(ip, &server_addr.sin_addr);

	if(connect(sock_fd, (const struct sockaddr *) &server_addr, sizeof(server_addr))==-1){
    #ifdef DEBUG
      printf("\tDEBUG: ERROR CONNECTING TO PEER %s:%s\n", ip, port);
    #endif
    close(sock_fd);
		return 0;
	}

  #ifdef DEBUG
    printf("\tDEBUG: RETURNING TCP SOCKET %d\n", sock_fd);
  #endif

  return sock_fd;
}


uint32_t gallery_add_photo(int peer_socket, char *file_name){

  char buff[100];
  int nbytes;
  char *token;
  char *previous;
  char f_name[100];
  strcpy(f_name, file_name);
  // OPENING FILE
  #ifdef DEBUG
    printf("\tDEBUG: OPENING FILE\n");
  #endif

  FILE *img = fopen(file_name, "rb");
  if(img == NULL){
    close(peer_socket);
    return 0;
  }


  // OPENING FILE
  #ifdef DEBUG
    printf("\tDEBUG: GETTING FILE CHARACTERISTICS\n");
  #endif



  // GET FILE CHARECTERISTICS
  fseek(img, 0, SEEK_END);
  size_t filesize = ftell(img);
  fseek(img, 0, SEEK_SET);

  #ifdef DEBUG
    printf("\tDEBUG: FINISHED GETTING FILE CHARECTERISTICS\n");
  #endif


  // PREPARING PROTOCOL MESSAGE TO PEER
   token = strtok(f_name, "/");

   /* walk through other tokens */
   while( token != NULL )
   {
      printf( " %s\n", token );
      previous = token;
      token = strtok(NULL, "/");
   }


  sprintf(buff, "ADDPHOTO %s %zu", previous, filesize);

  #ifdef DEBUG
    printf("\tDEBUG: SENDING MESSAGE TO PEER\n");
  #endif

  if(send(peer_socket, buff, strlen(buff)+1, 0)==-1){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
    #endif
    fclose(img);
    close(peer_socket);
    return 0;
  }

  #ifdef DEBUG
    printf("\tDEBUG: SENT TO PEER --- %s ---\n", buff);
  #endif

  nbytes = 0;
  nbytes = recv(peer_socket, buff, sizeof(buff), 0);
  if(nbytes <= 0){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT RECV MESSAGE FROM PEER\n");
    #endif
    fclose(img);
    close(peer_socket);
    return 0;
  }

  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
  #endif

  if(strcmp(buff,"OK")!=0){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT DECODE MESSAGE RECEIVED\n");
    #endif
    fclose(img);
    close(peer_socket);
    return 0;
  }

  // STORE READ DARA INTO BUFFER
  unsigned char *buffer = malloc(filesize);
  if(buffer == NULL){
    printf("COULD NOT ALLOCATE MEMORY\n");
    exit(-1);
  }
  fread(buffer, sizeof(char), filesize, img); // nota

  if(send(peer_socket, buffer, filesize, 0)==-1){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
    #endif
    fclose(img);
    close(peer_socket);
    free(buffer);
    return 0;
  }
  nbytes = 0;
  nbytes = recv(peer_socket, buff, sizeof(buff), 0);
  if(nbytes <= 0){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT RECV MESSAGE FROM PEER\n");
    #endif
    fclose(img);
    close(peer_socket);
    free(buffer);
    return 0;
  }

  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
  #endif

  char answer[10], photoid[10];
  sscanf(buff, "%s %s", answer, photoid);

  #ifdef DEBUG
    printf("\tDEBUG: DECODED FILEID %s\n", buff);
  #endif

  fclose(img);
  close(peer_socket);
  free(buffer);
  return strtoul(photoid, NULL, 0);

}




int gallery_add_keyword(int peer_socket, uint32_t id_photo, char *keyword){


  char query_buff[100], buff[100];
  int nbytes;

  // PREPARING PROTOCOL MESSAGE TO PEER
  sprintf(query_buff, "ADDKEY %d %s", id_photo, keyword);



  // SENDING MESSAGE TO PEER
  #ifdef DEBUG
    printf("\tDEBUG: SENDING MESSAGE TO PEER\n");
  #endif

  if(send(peer_socket, query_buff, strlen(query_buff)+1, 0)==-1){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
    #endif
    close(peer_socket);
    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: SENT TO PEER --- %s ---\n", query_buff);
  #endif



  // RECEIVING PEER RESPONSE
  nbytes = 0;
  nbytes = recv(peer_socket, buff, sizeof(buff), 0);
  if(nbytes <= 0){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT RECV MESSAGE FROM PEER\n");
    #endif
    close(peer_socket);
    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
  #endif



  //DECODING PEER RESPONSE
  if(strcmp(buff, "OK")==0){

    #ifdef DEBUG
      printf("\tDEBUG: ALL OK\n");
    #endif

    close(peer_socket);
    return 1;

  }

  if(strcmp(buff, "ERROR")==0){

    #ifdef DEBUG
      printf("\tDEBUG: PHOTO NOT FOUND\n");
    #endif

    close(peer_socket);
    return 0;
  }

  #ifdef DEBUG
    printf("COULD NOT DECODE MESSAGE\n");
  #endif

  close(peer_socket);
  return -1;

}




int gallery_search_photo(int peer_socket, char * keyword, uint32_t ** id_photo){

  char query_buff[100], buff[100], answer[10];
  int num_photo_ids;
  int nbytes;

  // PREPARING PROTOCOL MESSAGE TO PEER
  sprintf(query_buff, "SEARCH %s", keyword);



  // SENDING MESSAGE TO PEER
  #ifdef DEBUG
    printf("\tDEBUG: SENDING MESSAGE TO PEER\n");
  #endif

  if(send(peer_socket, query_buff, strlen(query_buff)+1, 0)==-1){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
    #endif
    close(peer_socket);
    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: SENT TO PEER --- %s ---\n", query_buff);
  #endif



  // RECEIVING PEER RESPONSE
  nbytes = 0;
  nbytes = recv(peer_socket, buff, sizeof(buff), 0);
  if(nbytes <= 0){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT RECV MESSAGE FROM PEER\n");
    #endif
    close(peer_socket);
    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
  #endif



  //DECODING PEER RESPONSE
  if(strcmp(buff, "ERROR")==0){

    #ifdef DEBUG
      printf("\tDEBUG: PHOTO NOT FOUND\n");
    #endif

    close(peer_socket);
    return 0;

  }

  uint32_t act_id;
  int k = 0;
  char *token;

  sscanf(buff, "%s %d", answer, &num_photo_ids);
  *id_photo = (uint32_t*)malloc(sizeof(uint32_t)*num_photo_ids);
  if(id_photo == NULL){
    printf("COULD NOT ALLOCATE MEMORY\n");
    exit(-1);
  }
  if(strcmp(answer, "OK")==0){

    #ifdef DEBUG
      printf("\tDEBUG: ALL OK. DECONDING PHOTO_IDs NOW\n");
    #endif

    nbytes = recv(peer_socket, buff, sizeof(buff), 0);
    if(nbytes <= 0){
      #ifdef DEBUG
        printf("\tDEBUG: COULD NOT RECV MESSAGE FROM PEER\n");
      #endif
      close(peer_socket);
      return -1;
    }
    char *sids = buff + 4;
    #ifdef DEBUG
      printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, sids);
    #endif

    token = strtok(sids, " ");

   /* walk through other tokens */
    while( token != NULL ) {
      //printf( " %s\n", token );
      act_id = (uint32_t)atoi(token);
      (*id_photo)[k]=act_id;
      token = strtok(NULL, " ");
      k++;
    }


    /*#ifdef DEBUG
      printf("\tDEBUG: %d IDS --- %s ---\n", num_photo_ids, photo_ids);
    #endif*/

    // FAZER STRTOK COM PHOTO_IDS PARA IR BUSCAR CADA UM DELES
    // RESPONSE: OK <num_ids> <photo_id1> .. <photo_idn>

    close(peer_socket);
    return num_photo_ids;
  }


  #ifdef DEBUG
    printf("COULD NOT DECODE MESSAGE\n");
  #endif

  close(peer_socket);
  return -1;

}





int gallery_delete_photo(int peer_socket, uint32_t id_photo){


    char query_buff[100], buff[100];
    int nbytes;

    // PREPARING PROTOCOL MESSAGE TO PEER
    sprintf(query_buff, "DELETE %d", id_photo);



    // SENDING MESSAGE TO PEER
    #ifdef DEBUG
      printf("\tDEBUG: SENDING MESSAGE TO PEER\n");
    #endif

    if(send(peer_socket, query_buff, strlen(query_buff)+1, 0)==-1){
      #ifdef DEBUG
        printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
      #endif
      close(peer_socket);
      return -1;
    }

    #ifdef DEBUG
      printf("\tDEBUG: SENT TO PEER --- %s ---\n", query_buff);
    #endif



    // RECEIVING PEER RESPONSE
    nbytes = 0;
    nbytes = recv(peer_socket, buff, sizeof(buff), 0);
    if(nbytes <= 0){
      #ifdef DEBUG
        printf("\tDEBUG: COULD NOT RECV MESSAGE FROM PEER\n");
      #endif
      close(peer_socket);
      return -1;
    }

    #ifdef DEBUG
      printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
    #endif



    //DECODING PEER RESPONSE
    if(strcmp(buff, "OK")==0){

      #ifdef DEBUG
        printf("\tDEBUG: ALL OK\n");
      #endif

      close(peer_socket);
      return 1;

    }
    if(strcmp(buff, "ERROR")==0){

      #ifdef DEBUG
        printf("\tDEBUG: PHOTO NOT FOUND\n");
      #endif

      close(peer_socket);
      return 0;

    }

    #ifdef DEBUG
      printf("COULD NOT DECODE MESSAGE\n");
    #endif

    close(peer_socket);
    return -1;

}





int gallery_get_photo_name(int peer_socket, uint32_t id_photo, char **photo_name){

    char query_buff[100], buff[100], answer[10];
    char name[100];
    int nbytes;

    // PREPARING PROTOCOL MESSAGE TO PEER
    sprintf(query_buff, "GETNAME %d", id_photo);



    // SENDING MESSAGE TO PEER
    #ifdef DEBUG
      printf("\tDEBUG: SENDING MESSAGE TO PEER\n");
    #endif

    if(send(peer_socket, query_buff, strlen(query_buff)+1, 0)==-1){
      #ifdef DEBUG
        printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
      #endif
      close(peer_socket);
      return -1;
    }

    #ifdef DEBUG
      printf("\tDEBUG: SENT TO PEER --- %s ---\n", query_buff);
    #endif



    // RECEIVING PEER RESPONSE
    nbytes = 0;
    nbytes = recv(peer_socket, buff, sizeof(buff), 0);
    if(nbytes <= 0){
      #ifdef DEBUG
        printf("\tDEBUG: COULD NOT RECV MESSAGE FROM PEER\n");
      #endif
      close(peer_socket);
      return -1;
    }

    #ifdef DEBUG
      printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
    #endif



    //DECODING PEER RESPONSE
    if(strcmp(buff, "ERROR")==0){

      #ifdef DEBUG
        printf("\tDEBUG: PHOTO NOT FOUND\n");
      #endif

      close(peer_socket);
      return 0;

    }

    sscanf(buff, "%s %s", answer, name);
    if(strcmp(answer, "OK")==0){

      #ifdef DEBUG
        printf("\tDEBUG: PHOTO FOUND: %s\n", name);
      #endif

      strcpy(*photo_name, name);


      #ifdef DEBUG
        printf("\tDEBUG: CLOSING SOCKET. RETURNING %s\n", *photo_name);
      #endif
      close(peer_socket);
      return 1;

    }

    #ifdef DEBUG
      printf("COULD NOT DECODE MESSAGE\n");
    #endif

    close(peer_socket);
    return -1;


}




int gallery_get_photo(int peer_socket, uint32_t id_photo, char *file_name){


  char query_buff[100], buff[100];
  int nbytes;
  unsigned long filesize;
  char answer[10];
  char photo_name[100];


  // PREPARING PROTOCOL MESSAGE TO PEER
  sprintf(query_buff, "GETPHOTO %d", id_photo);



  // SENDING MESSAGE TO PEER
  #ifdef DEBUG
    printf("\tDEBUG: SENDING MESSAGE TO PEER\n");
  #endif

  if(send(peer_socket, query_buff, strlen(query_buff)+1, 0)==-1){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
    #endif
    close(peer_socket);
    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: SENT TO PEER --- %s ---\n", query_buff);
  #endif

  // RECEIVING PEER RESPONSE
  nbytes = 0;
  nbytes = recv(peer_socket, buff, sizeof(buff), 0);
  if(nbytes <= 0){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT RECV MESSAGE FROM PEER\n");
    #endif
    close(peer_socket);
    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes, buff);
  #endif



  // DECODING PEER RESPONSE
  // RESPONSE: OK <FILESIZE>
  if(strcmp(buff, "ERROR")==0){

    #ifdef DEBUG
      printf("\tDEBUG: PHOTO NOT FOUND\n");
    #endif
    close(peer_socket);
    return 0;

  }


  sscanf(buff, "%s %s %lu", answer, photo_name, &filesize);
  if(strcmp(answer, "OK")==0){
    FILE *img = fopen(file_name, "wb");
    if(img == NULL){
      printf("COULD NOT OPEN FILE\n");
      exit(-1);
    }

    unsigned char buffer[CHUNK_SIZE];

    int rcv_size=0;
    int act_rcv_size = 0;
    //nbytes = recv(peer_socket, buffer, filesize, 0);

    while(rcv_size < filesize){
      act_rcv_size=recv(peer_socket, buffer, CHUNK_SIZE, 0);
      rcv_size=act_rcv_size+rcv_size;
      fwrite(buffer,1,act_rcv_size,img);
    }
    close(peer_socket);
    fclose(img);
    return 1;
  }

  close(peer_socket);
  return -1;



}
