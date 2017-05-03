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

  printf("\t\tDEBUG: KEYWORD LIST:\n");
  if(list->list!=NULL)
    for(i=1, aux = list->list; aux != NULL; aux=aux->next, i++)
      printf("\t\t\tDEBUG: KEYWORD %d: %s\n", i, aux->key);
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
  photos->total = 0;

  return photos;

}



void add_photo(photo_list *photos, char *name, uint32_t id){

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



photo * search_photo_by_id(photo_list *photos, uint32_t id){
  photo *aux;

  if(photos->list!=NULL)
    for(aux = photos->list; aux != NULL; aux=aux->next)
      if(aux->id == id)
        return aux;

  return NULL;

}



int add_keyword_photo(photo_list *photos, uint32_t id, char *keyword){

  photo * target = search_photo_by_id(photos, id);
  if(target!=NULL){
    add_keyword_list(target->keywords, keyword);
    return 1;
  }

  return 0;
}



int get_photo_name(photo_list *photos, uint32_t id, char *name){

  photo * target = search_photo_by_id(photos, id);
  if(target!=NULL){
    strcpy(name, target->name);
    return 1;
  }

  return 0;
}



int delete_photo(photo_list *photos, uint32_t id){
  photo *actual = NULL;
  photo *previous = NULL;

  if(photos->list==NULL)
    return 0;

  if(photos->list->id == id){
    actual = photos->list;
    photos->list = actual->next;


  }else{
    for(actual = photos->list; actual != NULL && actual->id != id; previous = actual, actual=actual->next);

    if(actual==NULL)
      return 0;

    previous->next = actual->next;
  }

  free_keyword_list(actual->keywords);
  free(actual);

  photos->total--;
  return 1;

}



void print_photo_list(photo_list *list){

  photo *aux;

  printf("\t\tDEBUG: PHOTO LIST:\n");
  if(list->list!=NULL)
    for(aux = list->list; aux != NULL; aux=aux->next){
      printf("\t\tDEBUG: PHOTO ID: %d NAME: %s\n", aux->id, aux->name);
      print_keyword_list(aux->keywords);
    }
}



void free_photo_list(photo_list *list){

  if(list->list!=NULL){

    photo *actual, *previous;
    actual = list->list;

    while(actual!=NULL){
      previous = actual;
      actual = actual->next;
      free_keyword_list(previous->keywords);
      free(previous);
    }

  }

  free(list);

}




/*
int main(){

  photo_list * list = init_photo_list();
  add_photo(list, "Primeira", 9283);
  print_photo_list(list);

  printf("FREEING\n");
  int ret = delete_photo(list, 9283);
  add_photo(list, "Segunda", 9284);
  print_photo_list(list);
  add_photo(list, "Terceira", 9285);
  print_photo_list(list);

  add_keyword_photo(list, 9285, "buefixe");
  add_keyword_photo(list, 9285, "buelinda");
  add_keyword_photo(list, 9285, "buenice");
  print_photo_list(list);
  ret = delete_photo(list, 9285);
  printf("delete %d\n",ret);
  ret = delete_photo(list, 213);
  printf("delete %d\n",ret);
  print_photo_list(list);
  add_photo(list, "Quarta", 9286);
  add_keyword_photo(list, 9286, "buefixe");
  char name[100];
  get_photo_name(list,9286,name);
  printf("%s", name);
  print_photo_list(list);
  printf("PRINTING ret %d\n", ret);
  print_photo_list(list);



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
