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
  uint32_t **ids = (uint32_t**)malloc(sizeof(uint32_t**));
  p=atoi(port);
  int photo_id1;
  char photo_name[100];
  char keyword[20];
  char query[15];
  int ret;

  while (1) {
    printf("**************************************************************************************************\n*\n");
    printf("*        Options:\n*\n");
    printf("*            addphoto - add a photo to the database\n*\n");
    printf("*            getphoto - get a photo from the data base[you must provide the id and name]\n*\n");
    printf("*            getname - get the name of a photo[you must provide the id]\n*\n");
    printf("*            addkeyword - add a keyword to a photo[you must provide the keyword a photo id]\n*\n");
    printf("*            search - search for a photo by keyword[you must provide the keyword]\n*\n");
    printf("*            delete - delete a photo from the database[you must provide the id]\n*\n");
    printf("*            quit - exit the program\n*\n");
    printf("**************************************************************************************************\n\n");
    printf("Enter command: ");
    fscanf(stdin, "%s", query);
    getchar();
    if(strcmp(query, "addphoto")==0){
      printf("enter the photo name:\n");
      fscanf(stdin, "%s", photo_name);
      psock = gallery_connect(host, p);
      if(psock>0){
        photo_id1 = gallery_add_photo(psock, photo_name);
        if(photo_id1>0){
          printf("\nPhoto added successfully. Your photo ID is %d\n\n", photo_id1);
        }else{
          printf("\nInvalid file_name or problems in communication with the server\n\n");
        }
      }else if(psock==0){
        printf("\nNo peer is available\n\n");
      }else{
        printf("\nGateway cannot be accessed\n\n");
      }
      close(psock);
    }else if(strcmp(query, "getphoto")==0){
      printf("enter the id and name[by this order and spaced]:\n");
      fscanf(stdin, "%d %s", &photo_id1, photo_name);
      psock = gallery_connect(host, p);
      if(psock>0){
        ret = gallery_get_photo(psock, photo_id1, photo_name);
        if(ret==1){
          printf("\nPhoto received\n\n");
        }else if(ret==0){
          printf("\nPhoto does not exist\n\n");
        }else{
          printf("\ninvalid arguments or network problem\n\n");
        }
      }else if(psock==0){
        printf("\nNo peer is available\n\n");
      }else{
        printf("\nGateway cannot be accessed\n\n");
      }
      close(psock);
    }else if(strcmp(query, "getname")==0){
      printf("enter the photo id:\n");
      fscanf(stdin, "%d", &photo_id1);
      psock = gallery_connect(host, p);
      if(psock>0){
        ret = gallery_get_photo_name(psock, photo_id1, name);
        if(ret==1){
          printf("\nPhoto with ID %d is named %s\n\n", photo_id1, (*name));
        }else if(ret==0){
          printf("\nPhoto does not exist\n\n");
        }else{
          printf("\ninvalid arguments or network problem\n\n");
        }
      }else if(psock==0){
        printf("\nNo peer is available\n\n");
      }else{
        printf("\nGateway cannot be accessed\n\n");
      }
      close(psock);
    }else if(strcmp(query, "addkeyword")==0){
      printf("enter the photo id and keyword to add[by this order and spaced]:\n");
      fscanf(stdin, "%d %s", &photo_id1, keyword);
      psock = gallery_connect(host, p);
      if(psock>0){
        ret = gallery_add_keyword(psock, photo_id1, keyword);
        if(ret==1){
          printf("\nKeyword added successfully\n\n");
        }else if(ret==0){
          printf("\nPhoto does not exist\n\n");
        }else{
          printf("\ninvalid arguments or network problem\n\n");
        }
      }else if(psock==0){
        printf("\nNo peer is available\n\n");
      }else{
        printf("\nGateway cannot be accessed\n\n");
      }
      close(psock);
    }else if(strcmp(query, "search")==0){
      printf("enter the keyword:\n");
      fscanf(stdin, "%s", keyword);
      psock = gallery_connect(host, p);
      if(psock>0){
        ret = gallery_search_photo(psock, keyword, ids);
        if (ret==0) {
          printf("\nNo photo contains the provided keyword\n\n");
        }else if (ret==-1) {
          printf("\nInvalid arguments or network problem\n\n");
        }else{
          printf("\nFound photos with ids ");
          for(i=0;i<ret;i++){
            printf("%u, ", (*ids)[i]);
          }
          printf("\n\n");
        }
      }else if(psock==0){
        printf("\nNo peer is available\n\n");
      }else{
        printf("\nGateway cannot be accessed\n\n");
      }
      close(psock);
    }else if(strcmp(query, "delete")==0){
      printf("enter the photo id:\n");
      fscanf(stdin, "%d", &photo_id1);
      psock = gallery_connect(host, p);
      if(psock>0){
        ret = gallery_delete_photo(psock, photo_id1);
        if(ret==1){
          printf("Photo deleted\n");
        }else if(ret==0){
          printf("\nPhoto does not exist\n\n");
        }else{
          printf("\ninvalid arguments or network problem\n\n");
        }
      }else if(psock==0){
        printf("\nNo peer is available\n\n");
      }else{
        printf("\nGateway cannot be accessed\n\n");
      }
      close(psock);
    }else if(strcmp(query, "quit")==0){
      printf("Goodbye\n");
      return 0;
    }else{
      printf("\n\"%s\" command is not valid\n\n", query);
    }
    strcpy(query, "");
  }
  free(name);
  free(*name);
  free(ids);


  return 0;
}
