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
#include "photostorageapi.h"

int main(int argc, char const *argv[]) {
  char *host, *port;
  port = calloc(128, sizeof(char));
  host = calloc(128, sizeof(char));
  in_port_t p;
  int psock;
  int i;
  if(argc!=5) { /* Number of arguments inserted is not correct */
      printf("Something went wrong...\nUsage: schat -n <name>.<survame> -i <ip> -p <scport> -s <snpip> -q <snpport>\n");
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

              default:
                      printf("Something went wrong...\nUsage: schat -n <name>.<survame> -i <ip> -p <scport> -s <snpip> -q <snpport>\n");
                      exit(-1);
                      break;
              }
      }
  }
  char **name = (char**)malloc(sizeof(char*));
  *name = (char*)malloc(100);
  uint32_t **ids = (uint32_t**)malloc(sizeof(uint32_t*));
  p=atoi(port);
  int photo_id1,photo_id2,photo_id3, photo_id;

  psock = gallery_connect(host, p);
  if(psock>0){
    photo_id1 = gallery_add_photo(psock, "5001.png");
  }
  getchar();


  psock = gallery_connect(host, p);
  if(psock>0){
    photo_id = gallery_add_photo(psock, "teste.png");
  }
  getchar();

  psock = gallery_connect(host, p);
  if(psock>0){
    photo_id2 = gallery_add_photo(psock, "5002.png");
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    photo_id3 = gallery_add_photo(psock, "5003.png");
  }

  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_get_photo_name(psock, photo_id1, name);
  }
  printf("printing name:\n");
  printf("name is %s\n", (*name));
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_get_photo_name(psock, photo_id1, name);
  }
  printf("printing name:\n");
  printf("name is %s\n", (*name));
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_get_photo(psock, 2, "teste.png");
    gallery_get_photo_name(psock, photo_id2, name);
  }
  printf("printing name:\n");
  printf("name is %s\n", (*name));
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_get_photo_name(psock, photo_id3, name);
  }
  printf("printing name:\n");
  printf("name is %s\n", (*name));
  getchar();

  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_add_keyword(psock, photo_id1, "PRAIA");
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_add_keyword(psock, photo_id2, "PLANICIE");
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_add_keyword(psock, photo_id3, "PLANICIE");
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_add_keyword(psock, photo_id1, "TORRE");
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_add_keyword(psock, photo_id1, "MONTANHA");
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_add_keyword(psock, photo_id3, "MONTANHA");
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_add_keyword(psock, photo_id2, "MONTANHA");
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_add_keyword(psock, photo_id2, "TORRE");
  }

  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_search_photo(psock, "TORRE", ids);
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_search_photo(psock, "MONTANHA", ids);
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_search_photo(psock, "PRAIA", ids);
  }
  getchar();
  psock = gallery_connect(host, p);
  if(psock>0){
    gallery_search_photo(psock, "PLANICIE", ids);
  }

  getchar();

  psock = gallery_connect(host, p);

  if(psock>0){
    gallery_delete_photo(psock, photo_id1);
  }
  getchar();

  psock = gallery_connect(host, p);

  if(psock>0){
    gallery_delete_photo(psock, photo_id2);
  }
  getchar();

  psock = gallery_connect(host, p);

  if(psock>0){
    gallery_delete_photo(psock, photo_id);
  }
  close(psock);
  return 0;
}
