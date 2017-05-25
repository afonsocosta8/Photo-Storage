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
  int photo_id;

    psock = gallery_connect(host, p);
    if(psock>0){
      photo_id = gallery_add_photo(psock, "5000.png");
    }
    getchar();
    psock = gallery_connect(host, p);

    if(psock>0){
      gallery_get_photo_name(psock, photo_id, name);
    }
    printf("printing name:\n");
    printf("name is %s\n", (*name));
    getchar();

    psock = gallery_connect(host, p);

    if(psock>0){
      gallery_add_keyword(psock, photo_id, "teste");
    }
    getchar();

    /*psock = gallery_connect(host, p);

    if(psock>0){
      gallery_search_photo(psock, "teste", ids);
    }
    getchar();*/

    psock = gallery_connect(host, p);

    if(psock>0){
      gallery_delete_photo(psock, photo_id);
    }
  close(psock);
  return 0;
}
