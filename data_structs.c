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
#include "data_structs.h"



keyword_list *init_keyword_list(){

  keyword_list *keys = (keyword_list*)malloc(sizeof(keyword_list));

  keys->list = NULL;

  return keys;

}

void add_keyword_list(keyword_list *keys, char *key){

  keyword *new = (keyword*)malloc(sizeof(keyword));

  strcpy(new->key, key);
  new->next = NULL;

  if(keys->list == NULL){

    keys->list = new;

  }else{

    keyword * aux;
    for(aux = keys->list; aux->next!=NULL; aux=aux->next);
    aux->next = new;

  }
}

int search_keyword_list(keyword_list *list, char *word){
  keyword*aux;
  if(list->list!=NULL)
    for(aux = list->list; aux != NULL; aux=aux->next)
      if(strcmp(aux->key, word)==0)
        return 1;

  return 0;

}


void print_keyword_list(keyword_list *list){

  keyword *aux;
  int i;

  printf("\t\t\tDEBUG: KEYWORD LIST:\n");
  if(list->list!=NULL)
    for(i=1, aux = list->list; aux != NULL; aux=aux->next, i++)
      printf("\t\t\tDEBUG: KEYWORD %s\n", aux->key);
}

void free_keyword_list(keyword_list *list){

  if(list->list!=NULL){
    keyword *actual, *previous;
    actual = list->list;
    while(actual!=NULL){
      previous = actual;
      actual = actual->next;
      free(previous);
    }
  }
  free(list);

}





photo_list *init_photo_list(){

  photo_list *photos = (photo_list*)malloc(sizeof(photo_list));

  photos->list = NULL;

  return photos;

}

void add_photo_list(photo_list *photos, char *name, uint32_t id){

  photo *new = (photo*)malloc(sizeof(photo));


  strcpy(new->name, name);
  new->id       = id;
  new->keywords = init_keyword_list();
  new->next     = NULL;

  if(photos->list == NULL){

    photos->list = new;

  }else{

    photo * aux;
    for(aux = photos->list; aux->next!=NULL; aux=aux->next);
    aux->next = new;

  }
}


/*
int search_photo_list(photo_list *list, char *word){

  if(list->list!=NULL)
    for(aux = list->list; aux != NULL; aux=aux->next)
      if(strcmp(aux->key, word)==0)
        return 1;

  return 0;

}


void print_keyword_list(keyword_list *list){

  keyword *aux;
  int i;

  printf("\t\t\tDEBUG: KEYWORD LIST:\n");
  if(list->list!=NULL)
    for(i=1, aux = list->list; aux != NULL; aux=aux->next, i++)
      printf("\t\t\tDEBUG: KEYWORD %s\n", aux->key);
}

void free_keyword_list(keyword_list *list){

  if(list->list!=NULL){
    keyword *actual, *previous;
    actual = list->list;
    while(actual!=NULL){
      previous = actual;
      actual = actual->next;
      free(previous);
    }
  }
  free(list);

}
*/
























// GATEWAY


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
