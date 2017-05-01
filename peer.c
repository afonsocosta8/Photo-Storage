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

#define CHUNK_SIZE 512

typedef struct _args{

  int mp;
  int p;
  char h[20];

}args;

typedef struct _header{
  long	data_length;
} header;


void * handle_alive(void * arg){

  struct sockaddr_in local_addr;
	struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
	socklen_t size_addr;
	char buff[20];
	int nbytes;
  char host[20];
  int p,mp;
  int sock_fd;
  struct timeval tv;
  tv.tv_sec = 1;

  #ifdef DEBUG
    printf("\t\tDEBUG: HELLO IM A NEW THREAD...\n");
  #endif



  // GET ARGUMENTS
  args *arguments = (args*)arg;
  strcpy(host, arguments->h);
  p = arguments->p;
  mp = arguments->mp;
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
  //SETTING TIMEOUT ON SOCKET
  if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0){
    perror("ERROR SETTING SOCKET TIMEOUT\n");
    #ifdef DEBUG
      printf("\t\tDEBUG: COULD NOT SET SOCKET OPTS\n");
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
	local_addr.sin_port= (mp);
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
    printf("\t\tDEBUG: SENT %dB TO GATEWAY %s:%d --- %s ---\n", nbytes, inet_ntoa(server_addr.sin_addr), server_addr.sin_port, query_buff);
  #endif




  // RECEIVING MESSAGE FROM GATEWAY
  char get_gw_resp[25];
  nbytes = 0;
  nbytes = recv(sock_fd, get_gw_resp, 9, 0);
  if(nbytes<=0) {
    printf("GATEWAY DID NOT ANSWER\n");
    exit(-1);
  }
  #ifdef DEBUG
    printf("\t\tDEBUG: %dB RECV --- %s ---\n", nbytes,  get_gw_resp);
  #endif
  if(strcmp(get_gw_resp, "OK")!=0) {
    printf("ERROR ON RECEIVING FROM GATEWAY\n");
    exit(-1);
  }




  // CHANGING PEER SOCKET RCV TIMEOUT TO ANSWER UALIVE CALLS
  tv.tv_usec = 0;
  if(setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0){
    perror("ERROR SETTING SOCKET TIMEOUT\n");
    #ifdef DEBUG
      printf("\t\tDEBUG: COULD NOT SET SOCKET OPTS\n");
    #endif
    exit(-1);
  }



  // LISTENING TO UALIVE? QUERYS
  #ifdef DEBUG
    printf("\t\tDEBUG: LISTENING FOR UALIVE? QUERYS\n");
  #endif
  size_addr = sizeof(client_addr);
  while(1){
    nbytes = recvfrom(sock_fd, buff, 20, 0, (struct sockaddr *) & client_addr, &size_addr);
    #ifdef DEBUG
      printf("\t\tDEBUG: %dB RECV FROM %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), client_addr.sin_port,  buff);
    #endif
    if(strcmp(buff, "UALIVE?")!=0) {
      printf("UNEXPECTED MESSAGE\n");
    }else{
      printf("RECEIVED UALIVE FROM GW. ANSWERING...\n");
      sprintf(buff, "OK");
      nbytes = sendto(sock_fd, buff, strlen(buff)+1, 0, (const struct sockaddr *) &client_addr, sizeof(client_addr));
      #ifdef DEBUG
        printf("\t\tDEBUG: SENT %dB TO PEER %s:%d --- %s ---\n", nbytes, inet_ntoa(client_addr.sin_addr), client_addr.sin_port, buff);
      #endif
    }
  }

}




int main(int argc, char const *argv[]) {

  char port[10], host[20], myport[10];
  in_port_t p, mp;
  pthread_t thr_id;
  int psock;
  int i;
  int get_peer_fd;
  int nbytes;
  struct timeval tv;
  tv.tv_sec = 1;

  // DECODING INPUT ARGUMENTS
  if(argc!=7){
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
  if(setsockopt(get_peer_fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0){
    perror("ERROR SETTING SOCKET OPTS\n");
    #ifdef DEBUG
      printf("\tDEBUG: COULD NOT SET SOCKET OPTS\n");
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
    printf("RETRIEVING FILES FROM PEER\n");
    // TRANSFER FILES FROM OTHER PEER

  }
  #ifdef DEBUG
  else
    printf("NO PEERS AVAILABLE - REGISTERING PEER\n");
  #endif




  // NOW WE JUST NEED TO REGISTER TO GATEWAY
  #ifdef DEBUG
    printf("\tDEBUG: CREATING THREAD TO HANDLE REG TO GW AND UALIVE QUERYS\n");
  #endif

  args *arguments = (args*)malloc(sizeof(args));
  arguments->mp = mp;
  arguments->p = p;
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
  socklen_t size_addr;
  char client_query[100];

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

  while(1){

    #ifdef DEBUG
      printf("\tDEBUG: WAITING FOR CLIENTS...\n");
    #endif
    client_fd= accept(sock_fd, (struct sockaddr *) & client_addr, &size_addr);
    printf("ACCEPTED ONE CONNECTION FROM %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);


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

    close(client_fd);
    printf("closing connectin with client\n");
    exit(0);
    */
  }

  exit(0);
}
