#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include "data_structs.h"

typedef struct _args{

  struct sockaddr_in client_addr;
  peer_list * list;

}args;

void * handle_ticket(){

  struct sockaddr_in local_addr;
  struct sockaddr_in client_addr;
  socklen_t size_addr;
  int sock_fd;
  int nbytes;
  uint32_t photo_id=0;

  // CREATING SOCKET TO RECV MESSAGES
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock_fd == -1){
    perror("ERROR CREATING TICKET  SOCKET\n");
    exit(-1);
  }
  #ifdef DEBUG
    printf("\tDEBUG: - TICKETS - SOCKET No: %d\n\tDEBUG: BINDING...\n", sock_fd);
  #endif


  // ASSIGNING SOCKET TO ADDRESS
  local_addr.sin_family = AF_INET;
	local_addr.sin_port= htons(9002);
	local_addr.sin_addr.s_addr= INADDR_ANY;
  if(bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
    perror("ERROR BINDIND TICKET SOCKET");
    exit(-1);
  }
  printf("READY TO RECEIVE MESSAGES TICKETS\n");


  char recvd_message[100];
  char resp_buff[100];

  while(1){
    nbytes = recvfrom(sock_fd, recvd_message, 100, 0, (struct sockaddr *) &client_addr, &size_addr);

    printf("\n\nNEW CLIENT ON TICKET THREAD FROM %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    #ifdef DEBUG
      printf("\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),  recvd_message);
    #endif

    if(nbytes>0){

      // PEER IS ALIVE. PREPARING RESPONSE TO CLIENT
      if(strcmp(recvd_message, "GET PHOTOID")==0){

        #ifdef DEBUG
          printf("\tDEBUG: DECODED AS GET PHOTOID\n");
        #endif

        printf("SERVING CLIENT %s:%d WITH PHOTO ID %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), photo_id);
        sprintf(resp_buff, "OK %d", photo_id);
        nbytes = sendto(sock_fd, resp_buff, strlen(resp_buff)+1, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr));
        photo_id++;
        if(nbytes>0){

          #ifdef DEBUG
            printf("\t\tDEBUG: SENT %dB TO PEER %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), resp_buff);
          #endif

          // WAITING FOR PEER RESPONSE. TIMEOUT = 1sec
          #ifdef DEBUG
            printf("\t\tDEBUG: WAITING FOR PEER RESPONSE.\n");
          #endif
        }
      }
    }else{

      #ifdef DEBUG
        printf("\tDEBUG: DECODED AS GET PHOTOID\n");
      #endif

      printf("COULD NOT SERVE CLIENT %s:%d WITH PHOTO ID %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), photo_id);

    }
  }
}

void * handle_get(void * arg){

  // CONNECTIONS RELATED
  char resp_buff[100];
  int nbytes;
  int test_peer_fd;
  char test_peer_query[10];
  struct sockaddr_in peer_addr;
  socklen_t size_addr;
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = 500000;

  // GET ARGUMENTS
  args *arguments = (args*)arg;
  struct sockaddr_in client_addr = arguments->client_addr;
  peer_list *list = arguments->list;

  // SEARCHING FOR A PEER IN PEER LIST
  #ifdef DEBUG
    printf("\t\tDEBUG: SEARCHING FOR A PEER IN PEER LIST\n");
  #endif
  int port;
  char ip[16];
  char buff[100];
  int DONE=0;

  while(!DONE){

    // GETTING A PEER FROM PEER LIST
    if(get_peer(list, ip, &port)){

      // TESTING IF PEER IS ALIVE
      // PREPARING MESSAGE
      test_peer_fd = socket(AF_INET, SOCK_DGRAM, 0);
      if(test_peer_fd == -1){
        perror("ERROR CREATING SOCKET\n");
        #ifdef DEBUG
          printf("\t\tDEBUG: COULD NOT CREATE SOCKET\n");
        #endif
        return;
    	}
      if(setsockopt(test_peer_fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0){
        perror("ERROR SETTING SOCKET OPTS\n");
        #ifdef DEBUG
          printf("\t\tDEBUG: COULD NOT SET SOCKET OPTS\n");
        #endif
        return;
      }
      strcpy(test_peer_query, "UALIVE?");
      peer_addr.sin_family = AF_INET;
    	peer_addr.sin_port = port;
    	inet_aton(ip, &peer_addr.sin_addr);

      // SENDING UALIVE? TO PEER
      #ifdef DEBUG
        print_peer_list(list);
        printf("\t\tDEBUG: RETRIEVED %s:%d FROM PEER LIST\n\t\tDEBUG: SENDING UALIVE? TO PEER\n", ip, port);
      #endif
      nbytes = sendto(test_peer_fd, test_peer_query, strlen(test_peer_query)+1, 0, (const struct sockaddr *) &peer_addr, sizeof(peer_addr));
      if(nbytes!=-1){

        #ifdef DEBUG
          printf("\t\tDEBUG: SENT %dB TO PEER %s:%d --- %s ---\n", nbytes, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port), test_peer_query);
        #endif

        // WAITING FOR PEER RESPONSE. TIMEOUT = 1sec
        #ifdef DEBUG
          printf("\t\tDEBUG: WAITING FOR PEER RESPONSE.\n");
        #endif

        nbytes = recvfrom(test_peer_fd, buff, 100, 0, (struct sockaddr *) &peer_addr, &size_addr);

        #ifdef DEBUG
          printf("\t\tDEBUG: ENDED WAITING FOR PEER RESPONSE.\n");
          printf("\t\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port),  buff);
        #endif
        if(nbytes>0){

          // PEER IS ALIVE. PREPARING RESPONSE TO CLIENT
          #ifdef DEBUG
            printf("\t\tDEBUG: PEER  %s:%d IS ALIVE\n", ip, port);
          #endif

          printf("SERVING CLIENT %s:%d WITH PEER %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), ip, port);
          sprintf(resp_buff, "OK %s:%d", ip, port);
          DONE = 1;

        }else{

          #ifdef DEBUG
            printf("\t\tDEBUG: PEER  %s:%d NOT ALIVE. REMOVING FROM PEER LIST...\n", ip, port);
          #endif

          // PEER NOT ALIVE... NEED TO RETRIEVE ANOTHER PEER FROM PEER LIST AND REMV THIS ONE
          remove_peer(list, ip, port);
          print_peer_list(list);

          #ifdef DEBUG
            printf("\t\tRETRIEVING ANOTHER PEER...\n");
          #endif


        }
      }else{

        #ifdef DEBUG
          printf("\t\tDEBUG: PEER  %s:%d NOT ALIVE. REMOVING FROM PEER LIST...\n", ip, port);
        #endif

        // PEER NOT ALIVE... NEED TO RETRIEVE ANOTHER PEER FROM PEER LIST AND REMV THIS ONE
        remove_peer(list, ip, port);
        print_peer_list(list);

        #ifdef DEBUG
          printf("\t\tRETRIEVING ANOTHER PEER...\n");
        #endif


      }
      close(test_peer_fd);
    }else{

      #ifdef DEBUG
        printf("\t\tDEBUG: NO PEERS ON PEER LIST\n");
      #endif

      printf("NO PEERS TO SERVE CLIENT %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
      sprintf(resp_buff, "ERROR NO PEERS");
      DONE = 1;

    }
  }

  // INITIALIZE RESPONSE SOCKET
  int resp_fd = socket(AF_INET, SOCK_DGRAM, 0);

  // SENDING RESPONSE TO CLIENT
  nbytes = sendto(resp_fd, resp_buff, strlen(resp_buff)+1, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr));

  if(nbytes==-1){

    printf("COULD NOT SEND RESPONSE TO CLIENT");


  }else{

    #ifdef DEBUG
      printf("\t\tDEBUG: SENT %dB TO CLIENT %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), resp_buff);
    #endif

  }
  free(arguments);
  close(resp_fd);
  return;
}

void * handle_reg(void * arg){

  char *resp_buff;
  int total_peers;
  char ** existing_peers;
  int nbytes;

  // GET ARGUMENTS
  args *arguments = (args*)arg;
  struct sockaddr_in client_addr = arguments->client_addr;
  peer_list *list = arguments->list;



  // INITIALIZE RESPONSE SOCKET
  int resp_fd = socket(AF_INET, SOCK_DGRAM, 0);

  // SENDING RESPONSE
  #ifdef DEBUG
    printf("\t\tDEBUG: RETRIEVING ALL EXISTING PEERS\n");
  #endif

  existing_peers = get_all_peers(list, &total_peers);

  #ifdef DEBUG
    printf("\t\tDEBUG: EXISTING PEERS:\n");
    for(int i = 0; i<total_peers; i++)
      printf("\t\t\t%s\n", existing_peers[i]);
  #endif

  resp_buff = (char*)malloc(sizeof(char)*(22*total_peers)+3);
  sprintf(resp_buff, "OK");
  for(int i=0; i<total_peers; i++){
    sprintf(resp_buff+strlen(resp_buff), " %s", existing_peers[i]);
    free(existing_peers[i]);
  }

  nbytes = sendto(resp_fd, resp_buff, strlen(resp_buff)+1, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr));
  #ifdef DEBUG
    printf("\t\tDEBUG: SENT %dB TO CLIENT %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), resp_buff);
  #endif


  // ADDING PEER TO LIST
  #ifdef DEBUG
    printf("\t\tDEBUG: ADDIND PEER %s:%d TO LIST\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
  #endif
  printf("ADDIND PEER %s:%d TO LIST\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
  add_peer_list(list, inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
  #ifdef DEBUG
    printf("\t\tDEBUG: REGISTERING PEER OK\n");
    print_peer_list(list);
  #endif



  free(existing_peers);
  free(arguments);
  close(resp_fd);
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
  peer_list *list = init_peer_list();

  #ifdef DEBUG
    printf("\tDEBUG: CREATING SOCKET...\n");
  #endif

  // CREATING SOCKET TO RECV MESSAGES
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock_fd == -1){
    perror("ERROR CREATING SOCKER\n");
    exit(-1);
  }

  #ifdef DEBUG
    printf("\tDEBUG: SOCKET No: %d\n\tDEBUG: BINDING...\n", sock_fd);
  #endif

  // ASSIGNING SOCKET TO ADDRESS
  local_addr.sin_family = AF_INET;
	local_addr.sin_port= htons(9001);
	local_addr.sin_addr.s_addr= INADDR_ANY;
  if(bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1){
    perror("ERROR BINDIND");
    exit(-1);
  }
  printf("READY TO RECEIVE MESSAGES\n");

  // GETTING TICKET SERVER
  if(pthread_create(&thr_id, NULL, handle_ticket, NULL)!=0){
    printf("ERROR CREATING THREAD FOR TICKER SERVER\n");
    exit(-1);
  }


  while(1){
    nbytes = recvfrom(sock_fd, buff, 100, 0, (struct sockaddr *) &client_addr, &size_addr);


    printf("\n\nNEW CLIENT CONNECTED FROM %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    #ifdef DEBUG
      printf("\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),  buff);
    #endif

    if(nbytes>0){
      if(strcmp(buff, "GET PEER")==0){
        #ifdef DEBUG
          printf("\tDEBUG: DECODED AS GET PEER\n\tDEBUG: CREATING THREAD FOR CLIENT...\n");
        #endif

        args *arguments= (args*)malloc(sizeof(args));
        arguments->client_addr = client_addr;
        arguments->list = list;

        if(pthread_create(&thr_id, NULL, handle_get, arguments)!=0){
          printf("ERROR CREATING THREAD FOR CLIENT %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          exit(-1);
        }

        #ifdef DEBUG
          printf("\tDEBUG: THREAD CREATED\n");
        #endif

      }else if(strcmp(buff, "REG PEER")==0){
        #ifdef DEBUG
          printf("\tDEBUG: DECODED AS REG PEER\n");
        #endif

        args *arguments= (args*)malloc(sizeof(args));
        arguments->client_addr = client_addr;
        arguments->list = list;

        if(pthread_create(&thr_id, NULL, handle_reg, arguments)!=0){
          printf("ERROR CREATING THREAD FOR CLIENT %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          exit(-1);
        }

      }else{
        #ifdef DEBUG
          printf("\tDEBUG: RECV INVALID COMMAND\n");
        #endif

      }
    }else{
      printf("Closing gateway.\n");
      exit(-1);
    }
  }

  exit(0);

}
