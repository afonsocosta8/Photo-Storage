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
#include <sys/time.h>
#include <string.h>

#include "data_structs.h"

#define CHUNK_SIZE 512

typedef struct _args_regpeer{

  int mp;
  int p;
  char h[20];
  brother_list *list;

}args_regpeer;


typedef struct _args_client{

  int client_fd;
  photo_hash_table *table;
  brother_list * brothers;
  char *host;

}args_client;

typedef struct _header{
  long	data_length;
} header;

uint32_t get_photoid(char * host){

  int sock_fd;
  int nbytes;

  printf("CONTACTING GATEWAY TO GET A NEW PHOTO ID\n");

  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock_fd == -1){
    perror("ERROR CREATING SOCKET\n");
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT CREATE SOCKET TO RETRIEVE PHOTOID\n");
    #endif
    return -1;
  }

  #ifdef DEBUG
    printf("\tDEBUG: CREATED SOCKET %d TO RETRIVE A NEW PHOTOID\n", sock_fd);
  #endif


  char get_photoid_query[12];
  struct sockaddr_in gateway_addr;
  strcpy(get_photoid_query, "GET PHOTOID");
  gateway_addr.sin_family = AF_INET;
	gateway_addr.sin_port = htons(9002);
  inet_aton(host, &gateway_addr.sin_addr);

  #ifdef DEBUG
    printf("\tDEBUG: SENDING PHOTOID QUERY TO GATEWAY\n");
  #endif

  nbytes = sendto(sock_fd, get_photoid_query, strlen(get_photoid_query)+1, 0, (const struct sockaddr *) &gateway_addr, sizeof(gateway_addr));
  if(nbytes==-1){
    #ifdef DEBUG
      printf("\t\tDEBUG: GATEWAY UNAVAILABLE WHEN RETRIEVING A NEW PHOTOID\n");
    #endif
    return -1;
  }
  #ifdef DEBUG
    printf("\t\tDEBUG: SENT %dB TO GATEWAY %s:%d --- %s ---\n", nbytes, inet_ntoa(gateway_addr.sin_addr), ntohs(gateway_addr.sin_port), get_photoid_query);
  #endif




  // RECEIVING MESSAGE FROM GATEWAY
  char get_gw_resp[25];
  nbytes = 0;
  nbytes = recv(sock_fd, get_gw_resp, 9, 0);
  if(nbytes<=0) {
    printf("GATEWAY DID NOT ANSWER\n");
    return -1;
  }
  #ifdef DEBUG
    printf("\t\tDEBUG: %dB RECV --- %s ---\n", nbytes,  get_gw_resp);
  #endif
  char gw_code[10];
  uint32_t photoid;
  sscanf(get_gw_resp, "%s %d", gw_code, &photoid);

  close(sock_fd);
  return photoid;



}

void add_image_brother(int client_fd, uint32_t photo_id, unsigned long filesize){

  unsigned char *buffer = malloc(filesize);
  unsigned char auxbuffer[1000];
  char towrite[100];
  int rcv_size=0;
  int act_rcv_size = 0;
  int k = 0;
  int i;
  int j = 0;
  while(rcv_size < filesize-1000){
    act_rcv_size=recv(client_fd, auxbuffer, 1000, 0);
    rcv_size=act_rcv_size+rcv_size;
    j=0;
    for(i = 1000*k;i<1000*k + act_rcv_size;i++){
      buffer[i] = auxbuffer[j];
      j++;
    }
    k++;
  }
  if(rcv_size!=filesize){
    j=0;
    act_rcv_size=recv(client_fd, auxbuffer, 1000, 0);
    for(i = 1000*k;i<1000*k + act_rcv_size;i++){
      buffer[i] = auxbuffer[j];
      j++;
    }
  }



  sprintf(towrite, "%u", photo_id);
  FILE *img = fopen(towrite, "wb");

  fwrite(buffer,1,filesize,img);

  fclose(img);
  free(buffer);
}

uint32_t add_image(int client_fd, char *photo_name, unsigned long filesize, char *host, photo_hash_table *table){

  unsigned char *buffer = malloc(filesize);
  unsigned char auxbuffer[1000];
  char towrite[100];
  uint32_t photo_id;
  //sprintf(towrite, "%s", photo_name);
  //FILE *img = fopen(towrite, "wb");
  int rcv_size=0;
  int act_rcv_size = 0;
  int k = 0;
  int i;
  int j = 0;

  //recv(client_fd, buffer, filesize, 0);
  while(rcv_size < filesize-1000){
    act_rcv_size=recv(client_fd, auxbuffer, 1000, 0);
    rcv_size=act_rcv_size+rcv_size;
    j=0;
    for(i = 1000*k;i<1000*k + act_rcv_size;i++){
      buffer[i] = auxbuffer[j];
      j++;
    }
    k++;
  }
  if(rcv_size!=filesize){
    j=0;
    act_rcv_size=recv(client_fd, auxbuffer, 1000, 0);
    for(i = 1000*k;i<1000*k + act_rcv_size;i++){
      buffer[i] = auxbuffer[j];
      j++;
    }
  }


  photo_id=get_photoid(host);
  sprintf(towrite, "%u", photo_id);
  FILE *img = fopen(towrite, "wb");

  fwrite(buffer,1,filesize,img);

  fclose(img);

  add_photo_hash_table(table, photo_name, photo_id);
  free(buffer);
  return photo_id;
}

uint32_t delete_image(int client_fd, uint32_t photo_id, photo_hash_table *table){

  int ret;
  printf("id=%d\n", photo_id);
  char todelete[20];
  print_photo_hash(table);

  sprintf(todelete, "%u", photo_id);
  ret = remove(todelete);

  if(ret == 0){
    int r = delete_photo_hash(table, photo_id);
    if(r!=1)
      printf("still in the database\n");
    printf("File deleted successfully\n");
    return 1;
  }
  else{
    printf("Error: unable to delete the file\n");
    return -1;
  }



  print_photo_hash(table);

}

int get_photo(int client_fd, uint32_t photo_id, photo_hash_table *table){

  char buff[100];

  // OPENING FILE
  #ifdef DEBUG
    printf("\tDEBUG: OPENING FILE\n");
  #endif

  char photo_name[100], file_name[110];
  if(get_photo_name_hash(table, photo_id, photo_name)==0){
    return -1;
  }

  #ifdef DEBUG
    printf("DEBUG: GETTING PHOTO NAME: %s\n", photo_name);
  #endif

  sprintf(file_name, "%d", photo_id);

  FILE *img = fopen(file_name, "rb");
  if(img == NULL)
    return 0;


  // GET FILE CHARECTERISTICS
  fseek(img, 0, SEEK_END);
  size_t filesize = ftell(img);
  fseek(img, 0, SEEK_SET);

  sprintf(buff, "OK %s %zu", photo_name, filesize);

  #ifdef DEBUG
    printf("\tDEBUG: SENDING MESSAGE TO PEER\n");
  #endif

  if(send(client_fd, buff, strlen(buff)+1, 0)==-1){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
    #endif
    fclose(img);
    close(client_fd);
    return 0;
  }
  #ifdef DEBUG
    printf("\tDEBUG: SENT TO PEER --- %s ---\n", buff);
  #endif

  unsigned char *buffer = malloc(filesize);
  fread(buffer, sizeof *buffer, filesize, img);

  if(send(client_fd, buffer, filesize, 0)==-1){
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SEND MESSAGE TO PEER\n");
    #endif
    fclose(img);
    close(client_fd);
    return 0;
  }

  free(buffer);
  return 1;
}

int search_ids(int client_fd, char *keyword, keyword_list *ids_list, photo_hash_table *table){
  int num_ids = 0;
  char id[20];
  photo *aux;
  key_word *aux1;
  int i, j;
  for(i=0; i<table->size; i++){
    if(table->table[i]->list!=NULL)
      for(aux = table->table[i]->list; aux != NULL; aux=aux->next){
        if(aux->keywords->list!=NULL)
          for(j=1, aux1 = aux->keywords->list; aux1 != NULL; aux1=aux1->next, j++){
            printf("im here comparing %s with %s\n", aux1->key, keyword);
            if (strcmp(aux1->key, keyword)==0) {
              sprintf(id, "%d", aux->id);
              add_keyword_list(ids_list, id);
              num_ids = num_ids + 1;
              break;
            }
          }
      }
  }
  print_keyword_list(ids_list);
  return num_ids;
}

void * handle_client(void * arg){
  args_client *arguments = (args_client*)arg;
  char host[20];
  char brother_ip[20];
  int brother_port;
  int brother_sock;
  int total;
  char ** brothers;
  struct sockaddr_in brother_addr;
  struct sockaddr_in gw_address;
  int gw_sock;
  int i;
  char peerdead[30];

  strcpy(host, arguments->host);
  photo_hash_table *table = arguments->table;
  brother_list *brothers_list = arguments->brothers;
  int client_fd = arguments->client_fd;

  free(arguments);
  int nbytes;
  char client_query[100];
  char buff[200];
  char answer[10];
  nbytes = recv(client_fd, client_query, 100, 0);
  printf("received %d bytes --- %s ---\n", nbytes, client_query);

  if(strstr(client_query, "ADDPHOTO") != NULL) {

    char photo_name[30];
    unsigned long filesize;
    uint32_t photo_id;

    sscanf(client_query, "%s %s %lu", answer, photo_name, &filesize);

    sprintf(buff, "OK");
    nbytes = send(client_fd, buff, strlen(buff)+1, 0);
    printf("replying %d bytes\n", nbytes);

    photo_id = add_image(client_fd, photo_name, filesize, host, table);
    if (photo_id!=0) {



      brothers = get_all_brothers(brothers_list, &total);
      for(i = 0; i<total; i++){
        sscanf(brothers[i], "%s %d", brother_ip, &brother_port);

        #ifdef DEBUG
          printf("\t DEBUG: INFORMING %s %d TO ADD PHOTO\n", brother_ip, brother_port);
        #endif

        // CREATING SOCKET TO SEND MESSAGE
        #ifdef DEBUG
          printf("\tDEBUG: CREATING SOCKET...\n");
        #endif

        brother_sock= socket(AF_INET, SOCK_STREAM, 0);
        if(brother_sock == -1){
          perror("ERROR CREATING SOCKER\n");
          exit(-1);
        }

        // PREPARING TO SEND MESSAGE TO GATEWAY
        #ifdef DEBUG
          printf("\tDEBUG: PREPARING MESSAGE TO GATEWAY\n");
        #endif
        brother_addr.sin_family = AF_INET;
        brother_addr.sin_port = htons(brother_port);
        inet_aton(brother_ip, &brother_addr.sin_addr);

        if(connect(brother_sock, (const struct sockaddr *) &brother_addr, sizeof(brother_addr))==-1){
          #ifdef DEBUG
            printf("\tDEBUG: ERROR CONNECTING TO PEER %s:%d\n", brother_ip, brother_port);
          #endif
      		return 0;
      	}
        char file_name[100];
        sprintf(file_name, "%u", photo_id);
        FILE *img = fopen(file_name, "rb");
        fseek(img, 0, SEEK_END);
        size_t filesize = ftell(img);
        fseek(img, 0, SEEK_SET);
        unsigned char *buffer = (unsigned char *)malloc(filesize);
        fread(buffer, sizeof *buffer, filesize, img);
        sprintf(buff, "RPLADD %s %d %zu", photo_name, photo_id, filesize);

        nbytes = send(brother_sock, buff, strlen(buff)+1, 0);
        #ifdef DEBUG
          printf("\t\tDEBUG: SENT %dB TO BRTOHER --- %s ---\n", nbytes, buff);
        #endif
        if(nbytes==-1){
          #ifdef DEBUG
            printf("\tDEBUG: MESSAGE NOT SENT\n");
          #endif
        }
        nbytes = recv(brother_sock, buff, 3, 0);
        #ifdef DEBUG
          printf("\t\tDEBUG: RECIVED %dB FROM CLIENT --- %s ---\n", nbytes, buff);
        #endif
        if(send(brother_sock, buffer, filesize, 0)==-1){
          #ifdef DEBUG
            printf("\tDEBUG: COULD NOT SEND IMAGE TO PEER\n");
          #endif
          fclose(img);
          free(buffer);
        }

        close(brother_sock);
        free(brothers[i]);
        free(buffer);
        fclose(img);
      }
      if(total!=0){
        free(brothers);
      }

      sprintf(buff, "OK %d", photo_id);
      nbytes = send(client_fd, buff, strlen(buff)+1, 0);

      #ifdef DEBUG
        printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
      #endif




    }else{
      printf("ERROR. COULDN'T STORE PHOTO\n");
      sprintf(buff, "ERROR");
      nbytes = send(client_fd, buff, strlen(buff)+1, 0);
      printf("replying %d bytes\n", nbytes);
    }

  }else if(strstr(client_query, "ADDKEY") != NULL) {


    char keyword[50];
    #ifdef DEBUG
      printf("\t\tDEBUG: DECODED AS ADD KEYWORD\n");
    #endif

    uint32_t photo_id;
    sscanf(client_query, "%s %d %s", answer, &photo_id, keyword);

    #ifdef DEBUG
      printf("\t\tDEBUG: ADDING KEYWORD %s to PHOTO %d\n", keyword, photo_id);
    #endif

    if(add_keyword_photo_hash(table, photo_id, keyword)){

      brothers = get_all_brothers(brothers_list, &total);
      for(i = 0; i<total; i++){
        sscanf(brothers[i], "%s %d", brother_ip, &brother_port);

        #ifdef DEBUG
          printf("\t DEBUG: INFORMING %s %d TO ADD KEY\n", brother_ip, brother_port);
        #endif

        // CREATING SOCKET TO SEND MESSAGE
        #ifdef DEBUG
          printf("\tDEBUG: CREATING SOCKET...\n");
        #endif

        brother_sock= socket(AF_INET, SOCK_STREAM, 0);
        if(brother_sock == -1){
          perror("ERROR CREATING SOCKER\n");
          exit(-1);
        }

        // PREPARING TO SEND MESSAGE TO GATEWAY
        #ifdef DEBUG
          printf("\tDEBUG: PREPARING MESSAGE TO GATEWAY\n");
        #endif
        brother_addr.sin_family = AF_INET;
        brother_addr.sin_port = htons(brother_port);
        inet_aton(brother_ip, &brother_addr.sin_addr);

        if(connect(brother_sock, (const struct sockaddr *) &brother_addr, sizeof(brother_addr))==-1){
          #ifdef DEBUG
            printf("\tDEBUG: ERROR CONNECTING TO PEER %s:%d\n", brother_ip, brother_port);
            printf("\tDEBUG: INFORMING GW TO REMOVE THIS PEER\n");
          #endif

          gw_sock = socket(AF_INET, SOCK_DGRAM, 0);
          if(gw_sock == -1){
            perror("ERROR CREATING SOCKER\n");
            exit(-1);
          }

          sprintf(peerdead, "RMV %s %d", brother_ip, brother_port);
          gw_address.sin_family = AF_INET;
          gw_address.sin_port = htons(9002);
          inet_aton(host, &gw_address.sin_addr);
          nbytes = sendto(gw_sock, peerdead, strlen(peerdead)+1,0, (const struct sockaddr *) &gw_address, sizeof(gw_address));

          #ifdef DEBUG
            printf("\t\tDEBUG: SENT %dB TO GATEWAY %s:%d --- %s ---\n", nbytes, inet_ntoa(gw_address.sin_addr), ntohs(gw_address.sin_port), peerdead);
          #endif

          if(nbytes==-1){
            #ifdef DEBUG
              printf("\tDEBUG: MESSAGE NOT SENT\n");
            #endif
          }

      	}

        sprintf(buff, "RPLKEY %d %s", photo_id, keyword);

        nbytes = send(brother_sock, buff, strlen(buff)+1, 0);
        #ifdef DEBUG
          printf("\t\tDEBUG: SENT %dB TO BRTOHER --- %s ---\n", nbytes, buff);
        #endif
        if(nbytes==-1){
          #ifdef DEBUG
            printf("\tDEBUG: MESSAGE NOT SENT\n");
          #endif

          gw_sock = socket(AF_INET, SOCK_DGRAM, 0);
          if(gw_sock == -1){
            perror("ERROR CREATING SOCKER\n");
            exit(-1);
          }

          sprintf(peerdead, "RMV %s %d", brother_ip, brother_port);
          gw_address.sin_family = AF_INET;
          gw_address.sin_port = htons(9002);
          inet_aton(host, &gw_address.sin_addr);
          nbytes = sendto(gw_sock, peerdead, strlen(peerdead)+1,0, (const struct sockaddr *) &gw_address, sizeof(gw_address));

          #ifdef DEBUG
            printf("\t\tDEBUG: SENT %dB TO GATEWAY %s:%d --- %s ---\n", nbytes, inet_ntoa(gw_address.sin_addr), ntohs(gw_address.sin_port), peerdead);
          #endif

          if(nbytes==-1){
            #ifdef DEBUG
              printf("\tDEBUG: MESSAGE NOT SENT\n");
            #endif
          }

        }

        close(brother_sock);
        free(brothers[i]);

      }
      if(total!=0){
        free(brothers);
      }

      sprintf(buff, "OK");
      #ifdef DEBUG
        printf("\t\tDEBUG: KEYWORD SUCCESSFULLY ADDED\n");
        print_photo_hash(table);
      #endif



    }else{
      sprintf(buff, "ERROR");
      #ifdef DEBUG
        printf("\t\tDEBUG: COULD NOT FOUND PHOTO ID\n");
      #endif
    }

    nbytes = send(client_fd, buff, strlen(buff)+1, 0);

    #ifdef DEBUG
      printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
    #endif


  }else if(strstr(client_query, "SEARCH") != NULL) {

    char keyword[30];
    keyword_list *ids_list = init_keyword_list();
    sscanf(client_query, "%s %s", answer, keyword);
    int num_ids = search_ids(client_fd, keyword, ids_list, table);
    if(num_ids==0){

      sprintf(buff, "NO PHOTOS");
      #ifdef DEBUG
        printf("\t\tDEBUG: COULD NOT FOUND PHOTO ID\n");
      #endif
      nbytes = send(client_fd, buff, strlen(buff)+1, 0);

      #ifdef DEBUG
        printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
      #endif

    }else{
      char * buffer = malloc(12*num_ids);
      key_word *aux;
      sprintf(buffer, "OK ");
      if(ids_list->list!=NULL){
        for(aux=ids_list->list; aux!=NULL; aux=aux->next){
          sprintf(buffer+strlen(buffer), "%s ", aux->key);
        }
      }
      free_keyword_list(ids_list);

      nbytes = send(client_fd, buffer, strlen(buffer)+1, 0);

      #ifdef DEBUG
        printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buffer);
      #endif


    }

  }else if(strstr(client_query, "DELETE") != NULL) {

    uint32_t photo_id;
    sscanf(client_query, "%s %d", answer, &photo_id);
    int res = delete_image(client_fd, photo_id, table);

    if(res==-1){

      sprintf(buff, "ERROR");
      #ifdef DEBUG
        printf("\t\tDEBUG: COULD NOT FOUND PHOTO ID\n");
      #endif
      nbytes = send(client_fd, buff, strlen(buff)+1, 0);

      #ifdef DEBUG
        printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
      #endif

    }else{

      #ifdef DEBUG
        printf("\t\tDEBUG: DELETED\n");
      #endif


      brothers = get_all_brothers(brothers_list, &total);
      for(i = 0; i<total; i++){
        sscanf(brothers[i], "%s %d", brother_ip, &brother_port);

        #ifdef DEBUG
          printf("\t DEBUG: INFORMING %s %d TO DELETE\n", brother_ip, brother_port);
        #endif

        // CREATING SOCKET TO SEND MESSAGE
        #ifdef DEBUG
          printf("\tDEBUG: CREATING SOCKET...\n");
        #endif

        brother_sock= socket(AF_INET, SOCK_STREAM, 0);
        if(brother_sock == -1){
          perror("ERROR CREATING SOCKER\n");
          exit(-1);
        }

        // PREPARING TO SEND MESSAGE TO GATEWAY
        #ifdef DEBUG
          printf("\tDEBUG: PREPARING MESSAGE TO GATEWAY\n");
        #endif
        brother_addr.sin_family = AF_INET;
        brother_addr.sin_port = htons(brother_port);
        inet_aton(brother_ip, &brother_addr.sin_addr);

        if(connect(brother_sock, (const struct sockaddr *) &brother_addr, sizeof(brother_addr))==-1){
          #ifdef DEBUG
            printf("\tDEBUG: ERROR CONNECTING TO PEER %s:%d\n", brother_ip, brother_port);
          #endif
      		return 0;
      	}

        sprintf(buff, "RPLDELETE %d", photo_id);

        nbytes = send(brother_sock, buff, strlen(buff)+1, 0);
        #ifdef DEBUG
          printf("\t\tDEBUG: SENT %dB TO BRTOHER --- %s ---\n", nbytes, buff);
        #endif
        if(nbytes==-1){
          #ifdef DEBUG
            printf("\tDEBUG: MESSAGE NOT SENT\n");
          #endif
        }

        close(brother_sock);
        free(brothers[i]);

      }
      if(total!=0){
        free(brothers);
      }

      sprintf(buff, "OK");
      nbytes = send(client_fd, buff, strlen(buff)+1, 0);

      #ifdef DEBUG
        printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
      #endif
    }

  }else if(strstr(client_query, "GETNAME") != NULL) {

    uint32_t photo_id;
    char name[100];
    sscanf(client_query, "%s %d", answer, &photo_id);


    #ifdef DEBUG
      printf("\t\tDEBUG: GETTING PHOTO'S %d NAME\n", photo_id);
    #endif

    if(get_photo_name_hash(table, photo_id, name)){
      sprintf(buff, "OK %s", name);
      #ifdef DEBUG
        printf("\t\tDEBUG: FOUND PHOTO - %s\n", name);
      #endif
    }else{
      sprintf(buff, "ERROR");
      #ifdef DEBUG
        printf("\t\tDEBUG: COULD NOT FOUND PHOTO ID\n");
      #endif
    }

    nbytes = send(client_fd, buff, strlen(buff)+1, 0);

    #ifdef DEBUG
      printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
    #endif


  }else if(strstr(client_query, "GETPHOTO") != NULL) {
    uint32_t photo_id;
    int res;
    sscanf(client_query, "%s %d", answer, &photo_id);
    res = get_photo(client_fd, photo_id, table);
    if(res!=1){
      sprintf(buff, "ERROR");
      nbytes = send(client_fd, buff, strlen(buff)+1, 0);
      printf("replying %d bytes\n", nbytes);
    }
  }else if(strstr(client_query, "RPLDLT") != NULL) {

    uint32_t photo_id;
    sscanf(client_query, "%s %d", answer, &photo_id);

    #ifdef DEBUG
      printf("\t\tDEBUG: DECODED AS RPL DELETE PHOTO %d\n", photo_id);
    #endif

    if(delete_image(client_fd, photo_id, table)==1){

      #ifdef DEBUG
        printf("\t\tDEBUG: DELETED PHOTO %d WITH SUCCESS\n", photo_id);
      #endif


    }else{

      #ifdef DEBUG
        printf("\t\tDEBUG: COULD NOT DELETE PHOTO %d\n", photo_id);
      #endif


    }

  }else if(strstr(client_query, "RPLKEY") != NULL) {


    char keyword[50];
    #ifdef DEBUG
      printf("\t\tDEBUG: DECODED AS RPL KEYWORD\n");
    #endif

    uint32_t photo_id;
    sscanf(client_query, "%s %d %s", answer, &photo_id, keyword);

    #ifdef DEBUG
      printf("\t\tDEBUG: ADDING KEYWORD %s to PHOTO %d\n", keyword, photo_id);
    #endif

    if(add_keyword_photo_hash(table, photo_id, keyword)){

      #ifdef DEBUG
        printf("\t\tDEBUG: ADDED KEYWORD %s to PHOTO %d WITH SUCESS\n", keyword, photo_id);
      #endif


    }else{

      #ifdef DEBUG
        printf("\t\tDEBUG: ADDING KEYWORD %s to PHOTO %d FAILED\n", keyword, photo_id);
      #endif
    }


  }else if(strstr(client_query, "RPLADD") != NULL) {



    size_t filesize;
    char photo_name[100];
    #ifdef DEBUG
      printf("\t\tDEBUG: DECODED AS RPL KEYWORD\n");
    #endif

    uint32_t photo_id;
    sscanf(client_query, "%s %s %d %zu", answer, photo_name, &photo_id, &filesize);

    #ifdef DEBUG
      printf("\t\tDEBUG: ADDING PHOTO %s WITH ID %d OF SIZE %zu\n", photo_name, photo_id, filesize);
    #endif

    sprintf(buff, "OK");
    nbytes = send(client_fd, buff, strlen(buff)+1, 0);
    #ifdef DEBUG
      printf("\t\tDEBUG: [4] SENT %dB TO BRTOHER --- %s ---\n", nbytes, buff);
    #endif
    if(nbytes==-1){
      #ifdef DEBUG
        printf("\tDEBUG: MESSAGE NOT SENT\n");
      #endif
    }

    add_image_brother(client_fd, photo_id, filesize);

    add_photo_hash_table(table, photo_name, photo_id);

    #ifdef DEBUG
      printf("\t\tDEBUG: ## ADDED A PHOTO TO PHOTO LIST\n");
      print_photo_hash(table);
    #endif



  }else if(strstr(client_query, "SHAREPHOTOS") != NULL) {

    #ifdef DEBUG
      printf("\t\tDEBUG: DECODED AS GET PHOTOS\n");
    #endif

    sprintf(buff, "OK %d", table->total);

    nbytes = send(client_fd, buff, strlen(buff)+1, 0);

    #ifdef DEBUG
      printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
    #endif

    unsigned char *buffer;
    size_t filesize;
    char file_name[20];
    photo *aux;
    key_word *aux1;
    int j;
    FILE *img;

    for(i=0; i<table->size; i++){
      if(table->table[i]->list!=NULL)
        for(aux = table->table[i]->list; aux != NULL; aux=aux->next){
          sprintf(file_name, "%u", aux->id);
          img = fopen(file_name, "rb");
          fseek(img, 0, SEEK_END);
          filesize = ftell(img);
          fseek(img, 0, SEEK_SET);
          buffer = (unsigned char *)malloc(filesize);
          fread(buffer, sizeof *buffer, filesize, img);
          sprintf(buff, "PHOTO %zu %u %s %d", filesize, aux->id, aux->name, aux->keywords->total);
          nbytes = send(client_fd, buff, strlen(buff)+1, 0);
          #ifdef DEBUG
            printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
          #endif
          nbytes = recv(client_fd, buff, 3, 0);
          #ifdef DEBUG
            printf("\t\tDEBUG: RECIVED %dB FROM CLIENT --- %s ---\n", nbytes, buff);
          #endif
          if(aux->keywords->total!=0){
            sprintf(buff, "KEYS");
            if(aux->keywords->list!=NULL)
              for(j=1, aux1 = aux->keywords->list; aux1 != NULL; aux1=aux1->next, j++){
                sprintf(buff, "%s %s", buff, aux1->key);
              }
            nbytes = send(client_fd, buff, strlen(buff)+1, 0);
            #ifdef DEBUG
              printf("\t\tDEBUG: SENT %dB TO CLIENT --- %s ---\n", nbytes, buff);
            #endif
            nbytes = recv(client_fd, buff, 3, 0);
            #ifdef DEBUG
              printf("\t\tDEBUG: RECIVED %dB FROM CLIENT --- %s ---\n", nbytes, buff);
            #endif
          }
          if(send(client_fd, buffer, filesize, 0)==-1){
            #ifdef DEBUG
              printf("\tDEBUG: COULD NOT SEND IMAGE TO PEER\n");
            #endif
            fclose(img);
            free(buffer);
          }
          nbytes = recv(client_fd, buff, 3, 0);
          #ifdef DEBUG
            printf("\t\tDEBUG: RECIVED %dB FROM CLIENT --- %s ---\n", nbytes, buff);
          #endif
          free(buffer);
          fclose(img);
        }
    }


  }else{

    #ifdef DEBUG
      printf("\t\tDEBUG: COULD NOT DECODE CLIENT QUERY\n");
    #endif


  }

  close(client_fd);
  return NULL;
}

void * handle_alive(void * arg){

  struct sockaddr_in local_addr;
	struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
	socklen_t size_addr;
	char buff[100];
	int nbytes;
  char host[20];
  int p,mp;
  brother_list * list;
  int sock_fd;

  #ifdef DEBUG
    printf("\t\tDEBUG: HELLO IM A NEW THREAD...\n");
  #endif



  // GET ARGUMENTS
  args_regpeer *arguments = (args_regpeer*)arg;
  strcpy(host, arguments->h);
  p = arguments->p;
  mp = arguments->mp;
  list = arguments->list;
  free(arguments);



  // REGISTERING TO GATEWAY
  // CREATING SOCKET
  #ifdef DEBUG
    printf("\t\tDEBUG: CREATING SOCKET\n");
  #endif
  sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sock_fd == -1){
    perror("ERROR CREATING SOCKET\n");
    #ifdef DEBUG
      printf("\t\tDEBUG: COULD NOT CREATE SOCKET\n");
    #endif
    exit(-1);
  }
  #ifdef DEBUG
    printf("\t\tDEBUG: SOCKET %d CREATED\n", sock_fd);
  #endif




  //BINDING SOCKET
  #ifdef DEBUG
    printf("\t\tDEBUG: BINDING SOCKET\n");
  #endif
	local_addr.sin_family = AF_INET;
	local_addr.sin_port= htons(mp);
	local_addr.sin_addr.s_addr= INADDR_ANY;
	int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
	if(err == -1) {
		perror("ERROR BINDING");
		exit(-1);
	}
  #ifdef DEBUG
    printf("\t\tDEBUG: SOCKET BINDED\n");
  #endif




  // SENDING REG PEER MESSAGE
  #ifdef DEBUG
    printf("\t\tDEBUG: SENDING MESSAGE TO GATEWAY...\n");
  #endif
  char query_buff[]="REG PEER";
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(p);
  inet_aton(host, &server_addr.sin_addr);
  nbytes = sendto(sock_fd, query_buff, strlen(query_buff)+1, 0, (const struct sockaddr *) &server_addr, sizeof(server_addr));
  if(nbytes==-1){
    #ifdef DEBUG
      printf("\t\tDEBUG: GATEWAY UNAVAILABLE.\n");
    #endif
    exit(-1);
  }
  #ifdef DEBUG
    printf("\t\tDEBUG: SENT %dB TO GATEWAY %s:%d --- %s ---\n", nbytes, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port), query_buff);
  #endif




  // RECEIVING MESSAGE FROM GATEWAY
  char get_gw_resp[25];
  char resp_code[4];
  char recv_code[5];
  char rmv_ip[20];
  int rmv_port;
  int num_peers;
  nbytes = 0;
  nbytes = recv(sock_fd, get_gw_resp, 9, 0);
  if(nbytes<=0) {
    printf("GATEWAY DID NOT ANSWER\n");
    exit(-1);
  }
  #ifdef DEBUG
    printf("\t\tDEBUG: %dB RECV --- %s ---\n", nbytes,  get_gw_resp);
  #endif
  sscanf(get_gw_resp, "%s %d", resp_code, &num_peers);
  if(strcmp(resp_code, "OK")!=0) {
    printf("ERROR ON RECEIVING FROM GATEWAY\n");
    exit(-1);
  }else{
    printf("THERE ARE %d ACTIVE PEERS\n", num_peers);
    if(num_peers!=0){
      char * active_peers = (char*)malloc((sizeof(char)*22*num_peers)+1);
      nbytes = 0;
      nbytes = recv(sock_fd, active_peers, 22*num_peers+1 , 0);
      if(nbytes<=0) {
        printf("GATEWAY DID NOT ANSWER\n");
        exit(-1);
      }
      #ifdef DEBUG
        printf("\t\tDEBUG: %dB RECV --- %s ---\n", nbytes,  active_peers);
      #endif
      char *brother_ip;
      int brother_port;
      #ifdef DEBUG
        printf("\t\tADDING BROTHER PEERS TO DATASTRUCT\n");
      #endif
      brother_ip = strtok(active_peers," ");

      while (brother_ip != NULL) {
        brother_port = atoi(strtok (NULL," "));
        add_brother_list(list, brother_ip, brother_port);
        #ifdef DEBUG
          printf("\t\tADDING %s:%d TO BROTHERS LIST\n", brother_ip, brother_port);
        #endif
        brother_ip = strtok(NULL," ");
      }
      #ifdef DEBUG
        printf("\t\tBROTHERS LIST:\n");
        print_brother_list(list);
      #endif
    }
  }


  // LISTENING TO UALIVE? QUERYS
  #ifdef DEBUG
    printf("\t\tDEBUG: LISTENING FOR UALIVE? QUERYS\n");
  #endif
  size_addr = sizeof(client_addr);

  while(1){

    nbytes = recvfrom(sock_fd, buff, 100, 0, (struct sockaddr *) & client_addr, &size_addr);
    #ifdef DEBUG
      printf("\t\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),  buff);
    #endif


    if(strcmp(buff, "UALIVE?")==0) {
      printf("RECEIVED UALIVE FROM GW. ANSWERING...\n");
      sprintf(buff, "OK");
      nbytes = sendto(sock_fd, buff, strlen(buff)+1, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr));
      #ifdef DEBUG
        printf("\t\tDEBUG: SENT %dB TO PEER %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buff);
      #endif
    }else{
      sscanf(buff, "%s %s %d", recv_code, rmv_ip, &rmv_port);
      if(strcmp(recv_code, "RMV")==0){
        #ifdef DEBUG
          printf("\t\tDECODED AS RMV PEER: REMOVING %s:%d\n", rmv_ip, rmv_port);
        #endif

        remove_brother(list, rmv_ip, rmv_port);

        #ifdef DEBUG
          printf("\t\tBROTHERS LIST: \n");
          print_brother_list(list);
        #endif


      }else if(strcmp(recv_code, "ADD")==0){
        #ifdef DEBUG
          printf("\t\tDECODED AS ADD PEER: ADDING %s:%d\n", rmv_ip, rmv_port);
        #endif

        add_brother_list(list, rmv_ip, rmv_port);

        #ifdef DEBUG
          printf("\t\tBROTHERS LIST: \n");
          print_brother_list(list);
        #endif

      }else{

        #ifdef DEBUG
          printf("\t\tCOULD NOT DECODE MESSAGE\n");
        #endif

      }

    }

  }

}




int main(int argc, char const *argv[]) {

  char port[10], host[20], myport[10];
  in_port_t p, mp;
  pthread_t thr_id;
  int i;
  int get_peer_fd;
  int nbytes;

  photo_hash_table * photos =  create_hash_table(769);
  brother_list * brothers = init_brother_list();


  // DECODING INPUT ARGUMENTS
  if(argc!=7){
      printf("Something went wrong...\nUsage: peer -h <gateway ip> -p <gateway port> <peer port>\n");
      exit(4);
  }else{ /*Reading each one of the arguments */
      for(i=1; i<argc; i=i+2) {
              switch(argv[i][1]) {

              case 'h':
                      strcpy(host, argv[i+1]);
                      break;

              case 'p':
                      strcpy(port, argv[i+1]);
                      break;

              case 'm':
                      strcpy(myport, argv[i+1]);
                      break;

              default:
                      printf("Something went wrong...\nUsage: schat -n <name>.<survame> -i <ip> -p <scport> -s <snpip> -q <snpport>\n");
                      exit(-1);
                      break;
              }
      }
  }
  #ifdef DEBUG
    printf("\tDEBUG: DECODED ARGS: %s %s %s\n", host, port, myport);
  #endif

  p=atoi(port);                 // THIS IS THE GATEWAY PORT
  mp=atoi(myport);              // THIS WILL BE THE PORT WHERE PEER WILL BE BINDED, HOST IS THE GATEWAY IP ADDRESS



  // FIRST PEER TASK IS TO RETRIEVE ANOTHER PEER IP ADDRESS TO DOWNLOAD ALL THE DATA
  // PREPARING MESSAGE

  printf("CONTACTING GATEWAY TO GET A PEER IP ADDRESS\n");

  get_peer_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(get_peer_fd == -1){
    perror("ERROR CREATING SOCKET\n");
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT CREATE SOCKET\n");
    #endif
    exit(-1);
  }
  #ifdef DEBUG
    printf("\tDEBUG: CREATED SOCKET %d TO RETRIVE PEER IP ADDRESS\n", get_peer_fd);
  #endif

  char get_peer_query[10];
  struct sockaddr_in gateway_addr;
  strcpy(get_peer_query, "GET PEER");
  gateway_addr.sin_family = AF_INET;
	gateway_addr.sin_port = htons(p);
	inet_aton(host, &gateway_addr.sin_addr);




  // SENDING MESSAGE TO GATEWAY
  #ifdef DEBUG
    printf("\tDEBUG: SENDING MESSAGE TO GATEWAY\n");
  #endif
	nbytes = sendto(get_peer_fd, get_peer_query, strlen(get_peer_query)+1, 0, (const struct sockaddr *) &gateway_addr, sizeof(gateway_addr));
	if(nbytes==-1) {
    printf("GATEWAY NOT AVAILABLE\n");
    exit(-1);
  }
  #ifdef DEBUG
    printf("\tDEBUG: SENT %dB TO CLIENT %s:%d --- %s ---\n", nbytes, inet_ntoa(gateway_addr.sin_addr), ntohs(gateway_addr.sin_port), get_peer_query);
  #endif




  // RECEIVING MESSAGE FROM GATEWAY
  char get_peer_resp[25];
  nbytes = 0;
  nbytes = recv(get_peer_fd, get_peer_resp, 25, 0);
  if(nbytes<=0) {
    printf("GATEWAY DID NOT ANSWER\n");
    exit(-1);
  }
  #ifdef DEBUG
    printf("\tDEBUG: %dB RECV --- %s ---\n", nbytes,  get_peer_resp);
  #endif
  close(get_peer_fd);




  // IN CASE THERE ARE ALREADY PEERS, I NEED TO TRANSFER FILES FROM THE OTHER PEER FRIST
  if(strcmp(get_peer_resp, "ERROR NO PEERS")!=0){
    char ipport[30];
    char code[5];
    char *brother_ip;
    int brother_port;
    struct sockaddr_in brother_addr;
    int brother_sock;
    char query[200];
    char resp_code[10];
    int num_photos;
    size_t file_size;
    int id;
    char name[100];
    int num_keys;
    char * keys;
    char * key;


    sscanf(get_peer_resp, "%s %s", code, ipport);
    if(strcmp(code, "OK")==0){

      #ifdef DEBUG
        printf("\tDEBUG: DECODED MESSAGE AS OK + PEER\n");
      #endif

      brother_ip = strtok(ipport,":");
      brother_port = atoi(strtok(NULL,":"));

      printf("RETRIEVING FILES FROM BROTHER %s:%d\n", brother_ip, brother_port);

      #ifdef DEBUG
        printf("\tDEBUG: CONNECTING TO BROTHER\n");
      #endif

      brother_sock= socket(AF_INET, SOCK_STREAM, 0);
      if(brother_sock == -1){
        perror("ERROR CREATING SOCKER\n");
        exit(-1);
      }

      // PREPARING TO SEND MESSAGE TO GATEWAY
      #ifdef DEBUG
        printf("\tDEBUG: PREPARING MESSAGE TO GATEWAY\n");
      #endif
      brother_addr.sin_family = AF_INET;
      brother_addr.sin_port = htons(brother_port);
      inet_aton(brother_ip, &brother_addr.sin_addr);

      if(connect(brother_sock, (const struct sockaddr *) &brother_addr, sizeof(brother_addr))==-1){
        #ifdef DEBUG
          printf("\tDEBUG: ERROR CONNECTING TO BRTOHER %s:%d\n", brother_ip, brother_port);
        #endif
        exit(-1);
      }

      sprintf(query, "SHAREPHOTOS");

      nbytes = send(brother_sock, query, strlen(query)+1, 0);
      #ifdef DEBUG
        printf("\t\tDEBUG: [1] SENT %dB TO BRTOHER --- %s ---\n", nbytes, query);
      #endif
      if(nbytes==-1){
        #ifdef DEBUG
          printf("\tDEBUG: MESSAGE NOT SENT\n");
        #endif
      }

      nbytes = recv(brother_sock, query, 200, 0);

      if(nbytes<=0) {
        printf("BRTOHER DID NOT ANSWER\n");
        exit(-1);
      }
      #ifdef DEBUG
        printf("\t\tDEBUG: [1] %dB RECV --- %s ---\n", nbytes,  query);
      #endif
      sscanf(query, "%s %d", resp_code, &num_photos);
      if(strcmp(resp_code, "OK")!=0) {
        printf("ERROR ON RECEIVING FROM BROTHER\n");
        exit(-1);
      }

      for(i=0; i<num_photos; i++){

        #ifdef DEBUG
          printf("\n\n\t\t\tDEBUG: RECEIVING PHOTO NUMBER %d\n", i);
        #endif

        nbytes = recv(brother_sock, query, 200, 0);

        if(nbytes<=0) {
          printf("BRTOHER DID NOT ANSWER\n");
          exit(-1);
        }
        #ifdef DEBUG
          printf("\t\tDEBUG: [2] %dB RECV --- %s ---\n", nbytes,  query);
        #endif

        sscanf(query, "%s %zu %d %s %d", resp_code, &file_size, &id, name, &num_keys);
        if(strcmp(resp_code, "PHOTO")!=0) {
          printf("ERROR ON RECEIVING FROM BROTHER\n");
          exit(-1);
        }

        #ifdef DEBUG
          printf("\t\tDEBUG: DECODED: FILESIZE=%zu | ID=%d | NAME=%s | NUMKEYS=%d\n", file_size, id, name, num_keys);
        #endif

        add_photo_hash_table(photos, name, id);

        #ifdef DEBUG
          printf("\t\tDEBUG: ## ADDED A PHOTO TO PHOTO LIST\n");
          print_photo_hash(photos);
        #endif



        sprintf(query, "OK");
        nbytes = send(brother_sock, query, strlen(query)+1, 0);
        #ifdef DEBUG
          printf("\t\tDEBUG: [2] SENT %dB TO BRTOHER --- %s ---\n", nbytes, query);
        #endif

        if(nbytes==-1){
          #ifdef DEBUG
            printf("\tDEBUG: MESSAGE NOT SENT\n");
          #endif
        }

        if(num_keys != 0){

          keys = (char *)malloc(sizeof(char)*22*num_keys);
          nbytes = recv(brother_sock, keys, 22*num_keys, 0);

          if(nbytes<=0) {
            printf("BRTOHER DID NOT ANSWER\n");
            exit(-1);
          }

          #ifdef DEBUG
            printf("\t\tDEBUG: [3] %dB RECV --- %s ---\n", nbytes,  query);
          #endif

          strtok(keys, " ");
          key = strtok(NULL, " ");
          while(key != NULL){
            add_keyword_photo_hash(photos, id, key);
            #ifdef DEBUG
              printf("\t\tDEBUG: ADDED A KEY %s TO PHOTO LIST\n", key);
              print_photo_hash(photos);
            #endif
            key = strtok(NULL, " ");
          }


          sprintf(query, "OK");
          nbytes = send(brother_sock, query, strlen(query)+1, 0);
          #ifdef DEBUG
            printf("\t\tDEBUG: [4] SENT %dB TO BRTOHER --- %s ---\n", nbytes, query);
          #endif
          if(nbytes==-1){
            #ifdef DEBUG
              printf("\tDEBUG: MESSAGE NOT SENT\n");
            #endif
          }

          free(keys);
        }




        #ifdef DEBUG
          printf("\t\tDEBUG: RECEIVING PHOTO\n");
        #endif

        add_image_brother(brother_sock, id, file_size);

        #ifdef DEBUG
          printf("\t\tDEBUG: FINISHED RECEIVING PHOTO\n");
        #endif

        sprintf(query, "OK");
        nbytes = send(brother_sock, query, strlen(query)+1, 0);
        #ifdef DEBUG
          printf("\t\tDEBUG: [4] SENT %dB TO BRTOHER --- %s ---\n", nbytes, query);
        #endif
        if(nbytes==-1){
          #ifdef DEBUG
            printf("\tDEBUG: MESSAGE NOT SENT\n");
          #endif
        }
      }
      close(brother_sock);
    }
  }

  #ifdef DEBUG
  else
    printf("NO PEERS AVAILABLE - REGISTERING PEER\n");
  #endif




  // NOW WE JUST NEED TO REGISTER TO GATEWAY
  #ifdef DEBUG
    printf("\tDEBUG: CREATING THREAD TO HANDLE REG TO GW AND UALIVE QUERYS\n");
  #endif

  args_regpeer *arguments = (args_regpeer*)malloc(sizeof(args_regpeer));
  arguments->mp = mp;
  arguments->p = p;
  arguments->list = brothers;
  strcpy(arguments->h, host);
  if(pthread_create(&thr_id, NULL, handle_alive, arguments)!=0){
    printf("ERROR CREATING THREAD FOR CLIENT\n");
    exit(-1);
  }

  #ifdef DEBUG
    printf("\tDEBUG: THREAD CREATED\n");
  #endif


  #ifdef DEBUG
    printf("\tDEBUG: PORT TO TCP STREAM: %d\n", mp);
  #endif



  // NEXT TASK: SETTING UP A TCP SERVER
  #ifdef DEBUG
    printf("\tDEBUG: SETTING UP TCP SERVER\n");
  #endif
  struct sockaddr_in local_addr;
  struct sockaddr_in client_addr;
  socklen_t size_addr = sizeof(client_addr);

  #ifdef DEBUG
    printf("\tDEBUG: CREATING TCP SOCKET\n");
  #endif
  int sock_fd= socket(AF_INET, SOCK_STREAM, 0);
  int client_fd;
  if(sock_fd == -1){
    printf("ERROR CREATING TCP SOCKET");
    exit(-1);
  }
  local_addr.sin_family = AF_INET;
  local_addr.sin_port = htons(mp);
  local_addr.sin_addr.s_addr= INADDR_ANY;
  #ifdef DEBUG
    printf("\tDEBUG: BINDING TCP SERVER\n");
  #endif

  int err = bind(sock_fd, (struct sockaddr *)&local_addr, sizeof(local_addr));
  if(err == -1) {
    perror("ERROR BINDING TCP SOCKET");
    exit(-1);
  }
  #ifdef DEBUG
    printf("\tDEBUG: LISTENING TCP SOCKET\n");
  #endif
  listen(sock_fd, 5);





  // READY TO ACCEPT CLIENTS
  printf("READY TO ACCEPT CLIENT CONNECTIONS\n");
  args_client *arguments1;
  while(1){


    arguments1 = (args_client*)malloc(sizeof(args_client));
    arguments1->table = photos;
    arguments1->host = host;
    arguments1->brothers = brothers;

    #ifdef DEBUG
      printf("\tDEBUG: WAITING FOR CLIENTS...\n");
    #endif
    client_fd= accept(sock_fd, (struct sockaddr *) & client_addr, &size_addr);
    printf("ACCEPTED ONE CONNECTION FROM %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

    arguments1->client_fd = client_fd;

    if(pthread_create(&thr_id, NULL, handle_client, arguments1)!=0){
      printf("ERROR CREATING THREAD FOR CLIENT\n");
      exit(-1);
    }

    #ifdef DEBUG
      printf("\tDEBUG: CREATED THREAD FOR CLIENT\n");
    #endif

    // NOW WE NEED TO ASSIGN THAT CLIENT TO A THREAD AND WAIT FOR HIS QUERY


    /*
    nbytes = recv(client_fd, client_query, 100, 0);
    printf("received %d bytes --- %s ---\n", nbytes, buff);


    sscanf("%s %s %d", answer, photo_name, filesize);

    sprintf(buff, "OK");
    nbytes = send(client_fd, buff, strlen(buff)+1, 0);
    printf("replying %d bytes\n", nbytes);

    unsigned char *buffer = malloc(filesize);

    nbytes = recv(client_fd, buffer, filesize, 0);

    sprintf(buff, "OK");
    nbytes = send(client_fd, buff, strlen(buff)+1, 0);
    printf("replying %d bytes\n", nbytes);

    fwrite(buffer,1,filesize,fp);

    printf("closing connectin with client\n");
    exit(0);
    */
  }

  free_brother_list(brothers);
  free_hash_table(photos);
  close(sock_fd);
  exit(0);
}
