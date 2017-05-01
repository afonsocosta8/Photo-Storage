#include "data_structs.h"

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
