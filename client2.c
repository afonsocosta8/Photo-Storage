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
  int photo_id1;
  char photo_name[100];
  char keyword[20];
  char query[15];

  while (1) {
    printf("Options:\n");
    printf("addphoto - add a photo to the database\n");
    printf("getphoto - get a photo from the data base[you must provide the id and name]\n");
    printf("getname - get the name of a photo[you must provide the id]\n");
    printf("addkeyword - add a keyword to a photo[you must provide the keyword a photo id]\n");
    printf("search - search for a photo by keyword[you must provide the keyword]\n");
    printf("delete - delete a photo from the database[you must provide the id]\n");
    fscanf(stdin, "%s", query);
    if(strcmp(query, "addphoto")==0){
      printf("enter the photo name:\n");
      fscanf(stdin, "%s", photo_name);
      psock = gallery_connect(host, p);
      if(psock>0){
        photo_id1 = gallery_add_photo(psock, photo_name);
      }
    }else if(strcmp(query, "getphoto")==0){
      printf("enter the id and name[by this order and spaced]:\n");
    }else if(strcmp(query, "getname")==0){
      printf("enter the photo id:\n");
      fscanf(stdin, "%d", &photo_id1);
      psock = gallery_connect(host, p);
      if(psock>0){
        gallery_get_photo_name(psock, photo_id1, name);
      }
      printf("printing name:\n");
      printf("name is %s\n", (*name));
    }else if(strcmp(query, "addkeyword")==0){
      printf("enter the photo id and keyword to add[by this order and spaced]:\n");
      fscanf(stdin, "%d %s", &photo_id1, keyword);
      psock = gallery_connect(host, p);
      if(psock>0){
        gallery_add_keyword(psock, photo_id1, keyword);
      }
    }else if(strcmp(query, "search")==0){
      printf("enter the keyword:\n");
      fscanf(stdin, "%s", keyword);
      psock = gallery_connect(host, p);
      if(psock>0){
        gallery_search_photo(psock, keyword, ids);
      }
    }else if(strcmp(query, "delete")==0){
      printf("enter the photo id:\n");
      fscanf(stdin, "%d", &photo_id1);
      psock = gallery_connect(host, p);
      if(psock>0){
        gallery_delete_photo(psock, photo_id1);
      }
    }else if(strcmp(query, "quit")==0){
      printf("Goodbye\n");
      close(psock);
      return 0;
    }
  }

  close(psock);


  return 0;
}
