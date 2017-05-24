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

  pthread_mutex_lock(&(photos->lock));
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

  pthread_mutex_unlock(&(photos->lock));

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


  pthread_mutex_lock(&(photos->lock));
  photo * target = search_photo_by_id(photos, id);
  if(target!=NULL){
    add_keyword_list(target->keywords, keyword);
    pthread_mutex_unlock(&(photos->lock));
    return 1;
  }
  pthread_mutex_unlock(&(photos->lock));

  return 0;
}



int get_photo_name(photo_list *photos, uint32_t id, char *name){


  pthread_mutex_lock(&(photos->lock));
  photo * target = search_photo_by_id(photos, id);
  if(target!=NULL){
    strcpy(name, target->name);
    pthread_mutex_unlock(&(photos->lock));
    return 1;
  }

  pthread_mutex_unlock(&(photos->lock));
  return 0;
}



int delete_photo(photo_list *photos, uint32_t id){
  photo *actual = NULL;
  photo *previous = NULL;

  pthread_mutex_lock(&(photos->lock));

  if(photos->list==NULL){
    pthread_mutex_unlock(&(photos->lock));
    return 0;
  }

  if(photos->list->id == id){
    actual = photos->list;
    photos->list = actual->next;


  }else{
    for(actual = photos->list; actual != NULL && actual->id != id; previous = actual, actual=actual->next);

    if(actual==NULL){
      pthread_mutex_unlock(&(photos->lock));
      return 0;
    }
    previous->next = actual->next;
  }

  free_keyword_list(actual->keywords);
  free(actual);

  photos->total--;
  pthread_mutex_unlock(&(photos->lock));
  return 1;

}




void print_photo_list(photo_list *list){

  photo *aux;
  pthread_mutex_lock(&(list->lock));
  if(list->list!=NULL)
    for(aux = list->list; aux != NULL; aux=aux->next){
      printf("\t\tDEBUG: PHOTO ID: %d NAME: %s\n", aux->id, aux->name);
      print_keyword_list(aux->keywords);
    }

  pthread_mutex_unlock(&(list->lock));
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

int get_hash_key(photo_hash_table *table, int id){

  return id % table->size;

}

photo_hash_table * create_hash_table(int size){

  photo_hash_table* new = malloc(sizeof(photo_hash_table));

  new->total = 0;
  new->size = size;
  new->table = malloc(sizeof(photo_list)*size);
  for(int i=0; i<size; i++)
    new->table[i] = init_photo_list();

  return new;

}

void free_hash_table(photo_hash_table *table){

  for(int i=0; i<table->size; i++)
    free_photo_list(table->table[i]);

  free(table->table);
  free(table);

}


void add_photo_hash_table(photo_hash_table *table, char *name, uint32_t id){

  add_photo(table->table[get_hash_key(table, id)], name, id);

}

int delete_photo_hash(photo_hash_table *table, uint32_t id){

  return delete_photo(table->table[get_hash_key(table, id)], id);

}


int add_keyword_photo_hash(photo_hash_table *table, uint32_t id, char *keyword){

  return add_keyword_photo(table->table[get_hash_key(table, id)], id, keyword);

}

int get_photo_name_hash(photo_hash_table *table, uint32_t id, char *name){

  return get_photo_name(table->table[get_hash_key(table, id)], id, name);

}

void print_photo_hash(photo_hash_table *table){

  printf("\t\tDEBUG: PHOTO LIST:\n");
  for(int i=0; i<table->size; i++)
    print_photo_list(table->table[i]);

}



/*
int main(){

  photo_hash_table * table = create_hash_table(769);
  add_photo_hash_table(table, "Primeira", 9283);
  print_photo_hash(table);
  add_photo_hash_table(table, "Segunda", 9284);
  print_photo_hash(table);
  add_photo_hash_table(table, "Terceira", 9285);
  print_photo_hash(table);

  add_keyword_photo_hash(table, 9285, "buefixe");
  add_keyword_photo_hash(table, 9285, "buelinda");
  add_keyword_photo_hash(table, 9285, "buenice");
  print_photo_hash(table);
  ret = delete_photo_hash(table, 9285);
  printf("delete %d\n",ret);
  ret = delete_photo_hash(table, 213);
  printf("delete %d\n",ret);
  print_photo_hash(table);
  add_photo_hash_table(table, "Quarta", 9286);
  add_keyword_photo_hash(table, 9286, "buefixe");
  char name[100];
  get_photo_name_hash(table,9286,name);
  printf("%s", name);
  print_photo_hash(table);
  printf("PRINTING ret %d\n", ret);
  print_photo_hash(table);

  free_hash_table(table);

}
*/


// GATEWAY


peer_list *init_peer_list(){

  peer_list *list = (peer_list*)malloc(sizeof(peer_list));

  list->next_to_use = NULL;
  list->beginning = NULL;
  list->total = 0;

  return list;

}

void add_peer_list(peer_list *list, char *ip, int port){


  pthread_mutex_lock(&(list->lock));

  peer *new = (peer*)malloc(sizeof(peer));

  strcpy(new->ip, ip);
  new->port = port;
  new->next = NULL;
  list->total++;

  if(list->beginning == NULL){

    list->beginning = new;
    new->next = new;

  }else{

    peer * aux;
    for(aux = list->beginning; aux->next != list->beginning; aux=aux->next);
    new ->next = list->beginning;
    aux->next = new;

  }

  pthread_mutex_unlock(&(list->lock));
}

void print_peer_list(peer_list *list){


  pthread_mutex_lock(&(list->lock));

  peer *aux;
  int i;

  printf("\t\t\tDEBUG: PEERS LIST TOTAL %d:\n", list->total);
  if(list->beginning!=NULL){
    if(list->beginning==list->beginning->next)
      printf("\t\t\tDEBUG: PEER 1: %s:%d\n", list->beginning->ip, list->beginning->port);

    else{
      for(i=1, aux = list->beginning; aux->next != list->beginning; aux=aux->next, i++)
        printf("\t\t\tDEBUG: PEER %d: %s:%d\n", i, aux->ip, aux->port);
      printf("\t\t\tDEBUG: PEER %d: %s:%d\n", i, aux->ip, aux->port);
    }
  }


  pthread_mutex_unlock(&(list->lock));
}

int get_peer(peer_list *list, char *ip, int *port){


  pthread_mutex_lock(&(list->lock));

  if(list->beginning == NULL){

    pthread_mutex_unlock(&(list->lock));
    return 0;

  }

  if(list->next_to_use==NULL)
    list->next_to_use = list->beginning;

  strcpy(ip, list->next_to_use->ip);
  *port = list->next_to_use->port;

  list->next_to_use = list->next_to_use->next;


  pthread_mutex_unlock(&(list->lock));
  return 1;

}


char** get_all_peers(peer_list *list, int* total){

  pthread_mutex_lock(&(list->lock));
  *total = list->total;

  if(list->beginning!=NULL){

    peer *aux;
    int i;
    char** peers = (char**)malloc(sizeof(char*)*list->total);


    for(i=0; i<list->total; i++)
      peers[i] = (char*)malloc(sizeof(char)*25);

    sprintf(peers[0], "%s %d", list->beginning->ip, list->beginning->port);
    if(list->beginning!=list->beginning->next){
      for(i=1, aux = list->beginning->next; aux != list->beginning; aux=aux->next, i++)
        sprintf(peers[i], "%s %d", aux->ip, aux->port);
    }
    pthread_mutex_unlock(&(list->lock));
    return peers;
  }

  pthread_mutex_unlock(&(list->lock));
  return NULL;
}

void remove_peer(peer_list *list, char *ip, int port){
  pthread_mutex_lock(&(list->lock));
  peer *actual;
  peer *previous;
  if(list->beginning!=NULL){
    for(actual = list->beginning;!(strcmp(ip, actual->ip)==0 && port==actual->port);  previous = actual, actual=actual->next);

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
      list->total--;
    }
    // else: we just need to free the element make the previous element pointing to the next
    else{

      previous->next = actual->next;
      free(actual);
      list->total--;

    }
  }
  pthread_mutex_unlock(&(list->lock));
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



// BROTHER LIST


brother_list *init_brother_list(){

  brother_list *list = (brother_list*)malloc(sizeof(brother_list));

  list->first = NULL;
  list->last = NULL;
  list->total = 0;
  return list;

}

void add_brother_list(brother_list *list, char *ip, int port){

  pthread_mutex_lock(&(list->lock));

  peer *new = (peer*)malloc(sizeof(peer));

  strcpy(new->ip, ip);
  new->port = port;
  new->next = NULL;

  if(list->first == NULL){

    list->first = new;
    list->last = new;

  }else{
    list->last->next=new;
    list->last = new;

  }

  list->total++;

  pthread_mutex_unlock(&(list->lock));
}

void print_brother_list(brother_list *list){

  pthread_mutex_lock(&(list->lock));
  peer *aux;
  int i;

  printf("\t\tDEBUG: BROTHER LIST TOTAL %d:\n", list->total);
  if(list->first!=NULL)
    for(i=0, aux = list->first; aux != NULL; aux=aux->next, i++)
      printf("\t\t\tDEBUG: PEER %d - %s:%d\n", i, aux->ip, aux->port);

  pthread_mutex_unlock(&(list->lock));
}


void remove_brother(brother_list *list, char *ip, int port){
  peer *actual;
  peer *previous;

  #ifdef DEBUG
    printf("\t\t\t\tDEBUG: LOCKING LIST\n");
  #endif
  pthread_mutex_lock(&(list->lock));

  if(list->first!=NULL){
    #ifdef DEBUG
      printf("\t\t\t\tDEBUG: SEARCHING FOR BROTHER\n");
    #endif
    for(actual = list->first; !(strcmp(ip, actual->ip)==0 && port==actual->port);  previous = actual, actual=actual->next);


    // first case: we want to remove the node that is the first of the list
    if(actual == list->first){
      #ifdef DEBUG
        printf("\t\t\t\tDEBUG: FIRST ELEMENT\n");
      #endif
      if(list->last == list->first){
        list->first = NULL;
        list->last = NULL;
      }else{
        list->first = actual->next;
        #ifdef DEBUG
          printf("\t\t\t\tDEBUG: FIRST ELEMENT EQUALS LAST ELEMENT\n");
        #endif
      }
      // free element we want to remove

    }
    // second case: we want to remove the last element
    else if (actual == list->last){
      #ifdef DEBUG
        printf("\t\t\t\tDEBUG: LAST ELEMENT\n");
      #endif
      previous->next = NULL;
      list->last = previous;
    }else{
      previous->next = actual->next;
    }
    #ifdef DEBUG
      printf("\t\t\t\tDEBUG: FREEING...\n");
    #endif
    free(actual);
    list->total--;
  }

  pthread_mutex_unlock(&(list->lock));
}

char **get_all_brothers(brother_list *list, int *total){


  pthread_mutex_lock(&(list->lock));

  *total = list->total;

  if(list->first!=NULL){
    peer *aux;
    int i;
    char **brothers = (char**)malloc(sizeof(char*)*list->total);


    for(i=0; i<list->total; i++)
      brothers[i] = (char*)malloc(sizeof(char)*25);

    if(list->first!=NULL)
      for(i=0, aux = list->first; aux != NULL; aux=aux->next, i++)
        sprintf(brothers[i], "%s %d", aux->ip, aux->port);

    pthread_mutex_unlock(&(list->lock));
    return brothers;


  }
  pthread_mutex_unlock(&(list->lock));
  return NULL;
}


void free_brother_list(brother_list *list){

  if(list->first!=NULL){

    peer *actual, *previous;
    actual = list->first;
    while(actual!=NULL){
      previous = actual;
      actual = actual->next;
      free(previous);
    }
  }

  free(list);

}


// BROTHER LIST TESTS

/*

int main(){

  peer_list * list = init_peer_list();

  printf("ADDING BROTHERS\n");
  add_peer_list(list, "127.0.0.1", 9012);
  add_peer_list(list, "127.0.0.2", 9013);
  add_peer_list(list, "127.0.0.3", 9014);
  add_peer_list(list, "127.0.0.4", 9015);
  add_peer_list(list, "127.0.0.5", 9016);

  printf("PRITING BROTHERS\n");
  print_peer_list(list);

  printf("REMOVING BROTHER - 127.0.0.3:9014\n");
  remove_peer(list, "127.0.0.3", 9014);

  printf("PRITING BROTHERS\n");
  print_peer_list(list);

  printf("REMOVING BROTHER - 127.0.0.1:9012 (FIRST)\n");
  remove_peer(list, "127.0.0.1", 9012);

  printf("PRITING BROTHERS\n");
  print_peer_list(list);

  printf("REMOVING BROTHER - 127.0.0.5:9016 (LAST)\n");
  remove_peer(list, "127.0.0.5", 9016);

  printf("PRITING BROTHERS\n");
  print_peer_list(list);

  printf("ADDING BROTHER\n");
  add_peer_list(list, "127.0.0.10", 9020);

  printf("PRITING BROTHERS\n");
  print_peer_list(list);

  printf("GETTING ALL BROTHERS\n");
  char **brothers;
  int total=0;
  brothers = get_all_peers(list, &total);
  for(int i = 0; i<total; i++){
    printf("%s\n", brothers[i]);
    free(brothers[i]);
  }

  free(brothers);

  printf("PRITING BROTHERS\n");
  print_peer_list(list);

  printf("FREEING BROTHER LIST\n");
  free_peer_list(list);

}*/
