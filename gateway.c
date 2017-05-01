#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>


typedef struct _peer{

  char ip[16];
  int  port;
  struct _peer *next;

}peer;


typedef struct _peerlist{

  peer *next_to_use;
  peer *beginning;

}peer_list;

typedef struct _args{

  struct sockaddr_in client_addr;
  peer_list * list;

}args;


peer_list *init_peer_list(){

  peer_list *list = (peer_list*)malloc(sizeof(peer_list));

  list->next_to_use = NULL;
  list->beginning = NULL;

  return list;

}

void add_peer_list(peer_list *list, char *ip, int port){

  peer *new = (peer*)malloc(sizeof(peer));

  strcpy(new->ip, ip);
  new->port = port;
  new->next = NULL;

  if(list->beginning == NULL){

    list->beginning = new;
    new->next = new;

  }else{

    peer * aux;
    for(aux = list->beginning; aux->next != list->beginning; aux=aux->next);
    new ->next = list->beginning;
    aux->next = new;

  }
}

void print_peer_list(peer_list *list){

  peer *aux;
  int i;

  printf("\t\t\tDEBUG: PEERS LIST:\n");
  if(list->beginning!=NULL){
    if(list->beginning==list->beginning->next)
      printf("\t\t\tDEBUG: PEER 1: %s:%d\n", list->beginning->ip, list->beginning->port);

    else{
      for(i=1, aux = list->beginning; aux->next != list->beginning; aux=aux->next, i++)
        printf("\t\t\tDEBUG: PEER %d: %s:%d\n", i, aux->ip, aux->port);
      printf("\t\t\tDEBUG: PEER %d: %s:%d\n", i, aux->ip, aux->port);
    }
  }
}

int get_peer(peer_list *list, char *ip, int *port){

  if(list->beginning == NULL)
    return 0;

  if(list->next_to_use==NULL)
    list->next_to_use = list->beginning;

  strcpy(ip, list->next_to_use->ip);
  *port = list->next_to_use->port;

  list->next_to_use = list->next_to_use->next;

  return 1;

}

void remove_peer(peer_list *list, char *ip, int port){
  peer *actual;
  peer *previous;
  if(list->beginning!=NULL){
    for(actual = list->beginning; (strcmp(ip, actual->ip)!=0 && port!=actual->port);  previous = actual, actual=actual->next);

    // first case: we want to remove the node that is the beginning of the list
    if(actual == list->beginning){
      peer* last;

      // find last element on the list
      for(last = list->beginning; last->next != list->beginning; last=last->next);
      // if last == begining, then there is only one element on the list, put beginning pointing to NULL
      if(last == list->beginning){
        list->beginning = NULL;
        list->next_to_use = NULL;
      }else{
        // put the begining and last element of the list pointing to the second element of the list
        last->next = list->beginning = actual->next;
      }
      // free element we want to remove
      free(actual);
    }
    // else: we just need to free the element make the previous element pointing to the next
    else{

      previous->next = actual->next;
      free(actual);

    }
  }
}

void free_peer_list(peer_list *list){

  if(list->beginning!=NULL){
    peer *actual;
    peer *last, *previous;
    actual = list->beginning;
    for(last = list->beginning; last->next != list->beginning; last=last->next);
    last->next = NULL;
    while(actual!=NULL){
      previous = actual;
      actual = actual->next;
      free(previous);
    }
  }
  free(list);

}

void * handle_get(void * arg){

  // CONNECTIONS RELATED
  char resp_buff[100];
  int nbytes;
  int test_peer_fd;
  char test_peer_query[10];
  struct sockaddr_in peer_addr;
  time_t start, end;
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
          printf("\t\tDEBUG: SENT %dB TO PEER %s:%d --- %s ---\n", nbytes, inet_ntoa(peer_addr.sin_addr), peer_addr.sin_port, test_peer_query);
        #endif

        // WAITING FOR PEER RESPONSE. TIMEOUT = 1sec
        #ifdef DEBUG
          printf("\t\tDEBUG: WAITING FOR PEER RESPONSE.\n");
        #endif

        nbytes = recvfrom(test_peer_fd, buff, 100, 0, (struct sockaddr *) &peer_addr, &size_addr);

        #ifdef DEBUG
          printf("\t\tDEBUG: ENDED WAITING FOR PEER RESPONSE.\n");
          printf("\t\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(peer_addr.sin_addr), peer_addr.sin_port,  buff);
        #endif
        if(nbytes>0){

          // PEER IS ALIVE. PREPARING RESPONSE TO CLIENT
          #ifdef DEBUG
            printf("\t\tDEBUG: PEER  %s:%d IS ALIVE\n", ip, port);
          #endif

          printf("SERVING CLIENT %s:%d WITH PEER %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, ip, port);
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

      printf("NO PEERS TO SERVE CLIENT %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
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
      printf("\t\tDEBUG: SENT %dB TO CLIENT %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), client_addr.sin_port, resp_buff);
    #endif

  }
  free(arguments);
  close(resp_fd);
  return;
}

void * handle_reg(void * arg){

  char resp_buff[100];
  int nbytes;

  // GET ARGUMENTS
  args *arguments = (args*)arg;
  struct sockaddr_in client_addr = arguments->client_addr;
  peer_list *list = arguments->list;
  struct timeval tv;
  tv.tv_sec = 1;

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

  // INITIALIZE RESPONSE SOCKET
  int resp_fd = socket(AF_INET, SOCK_DGRAM, 0);

  // SENDING RESPONSE
  sprintf(resp_buff, "OK");
  nbytes = sendto(resp_fd, resp_buff, strlen(resp_buff)+1, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr));
  #ifdef DEBUG
    printf("\t\tDEBUG: SENT %dB TO CLIENT %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), client_addr.sin_port, resp_buff);
  #endif

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


  while(1){
    nbytes = recvfrom(sock_fd, buff, 100, 0, (struct sockaddr *) &client_addr, &size_addr);


    printf("\n\nNEW CLIENT CONNECTED FROM %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
    #ifdef DEBUG
      printf("\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), client_addr.sin_port,  buff);
    #endif

    if(strcmp(buff, "GET PEER")==0){
      #ifdef DEBUG
        printf("\tDEBUG: DECODED AS GET PEER\n\tDEBUG: CREATING THREAD FOR CLIENT...\n");
      #endif

      args *arguments= (args*)malloc(sizeof(args));
      arguments->client_addr = client_addr;
      arguments->list = list;

      if(pthread_create(&thr_id, NULL, handle_get, arguments)!=0){
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

      args *arguments= (args*)malloc(sizeof(args));
      arguments->client_addr = client_addr;
      arguments->list = list;

      if(pthread_create(&thr_id, NULL, handle_reg, arguments)!=0){
        printf("ERROR CREATING THREAD FOR CLIENT %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);
        exit(-1);
      }

    }else{
      #ifdef DEBUG
        printf("\tDEBUG: RECV INVALID COMMAND\n");
      #endif

    }
  }

  exit(0);

}
