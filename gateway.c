#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



typedef struct _peer{

  char ip[16];
  int  port;
  struct _peer *next;

}peer;


typedef struct _peerlist{

  peer *last_used;
  peer *beginning;

}peer_list;

typedef struct _args{

  struct sockaddr_in client_addr;
  peer_list * list;

}args;


peer_list *init_peer_list(){

  peer_list *list = (peer_list*)malloc(sizeof(peer_list));

  list->last_used = NULL;
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

  printf("\tDEBUG: PEERS LIST:\n");
  if(list->beginning!=NULL){
    if(list->beginning==list->beginning->next)
      printf("\tDEBUG: PEER 1: %s:%d\n", list->beginning->ip, list->beginning->port);

    else{
      for(i=1, aux = list->beginning; aux->next != list->beginning; aux=aux->next, i++)
        printf("\tDEBUG: PEER %d: %s:%d\n", i, aux->ip, aux->port);
      printf("\tDEBUG: PEER %d: %s:%d\n", i, aux->ip, aux->port);
    }
  }
}

int get_peer(peer_list *list, char *ip, int *port){

  if(list->beginning == NULL)
    return 0;

  if(list->last_used==NULL)
    list->last_used = list->beginning;
  else
    list->last_used = list->last_used->next;

  strcpy(ip, list->last_used->ip);
  *port = list->last_used->port;

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
        list->last_used = NULL;
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

  char resp_buff[100];
  int nbytes;

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
  if(get_peer(list, ip, &port)){

    #ifdef DEBUG
      printf("\t\tDEBUG: RETRIEVED %s:%d FROM PEER LIST\n", ip, port);
    #endif
    sprintf(resp_buff, "OK %s:%d", ip, port);

  }else{



  }

  // INITIALIZE RESPONSE SOCKET
  int resp_fd = socket(AF_INET, SOCK_DGRAM, 0);
  printf("SERVING CLIENT %s:%d WITH PEER %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port, "192.168.2.2", 5000);

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

  // ADD PEER TO LIST

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
  peer_list *list = init_peer_list();

 /*
  char ip[20];
  int port;
  add_peer_list(list, "192.168.1.10", 8012);
  add_peer_list(list, "192.168.1.10", 8013);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  add_peer_list(list, "192.168.1.10", 8015);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  add_peer_list(list, "192.168.1.10", 8016);
  print_peer_list(list);
  remove_peer(list, "192.168.1.10", 8015);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  print_peer_list(list);
  remove_peer(list, "192.168.1.10", 8013);
  print_peer_list(list);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  remove_peer(list, "192.168.1.10", 8012);
  print_peer_list(list);
  remove_peer(list, "192.168.1.10", 8016);
  print_peer_list(list);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  add_peer_list(list, "192.168.1.10", 8012);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  add_peer_list(list, "192.168.1.10", 8013);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  add_peer_list(list, "192.168.1.10", 8015);
  add_peer_list(list, "192.168.1.10", 8016);
  print_peer_list(list);

  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  get_peer(list, ip, &port);
  printf("%s:%d\n",ip,port);
  free_peer_list(list);
*/



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
	local_addr.sin_port= htons(9001);
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

    printf("NEW CLIENT CONNECTED FROM %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

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



    }else{
      #ifdef DEBUG
        printf("\tDEBUG: RECV INVALID COMMAND\n");
      #endif


    }


  }

  exit(0);




}
