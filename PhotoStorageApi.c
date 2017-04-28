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

int gallery_connect(char * host, in_port_t p){

  struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char buff[]="GET";
  char port[6];
  char ip[20];
	int nbytes;

	int sock_fd= socket(AF_INET, SOCK_DGRAM, 0);

	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	printf(" socket to gateway created \n Ready to send GET message\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(p);
	inet_aton(host, &server_addr.sin_addr);

	nbytes = sendto(sock_fd,
	                    buff, strlen(buff)+1, 0,
	                    (const struct sockaddr *) &server_addr, sizeof(server_addr));
	printf("sent %d %s\n", nbytes, buff);
	nbytes = recv(sock_fd, buff, 20, 0);
  sscanf(buff, "OK %s:%s", ip, port);
	printf("received %d bytes --- ip = %s port = %s ---\n", nbytes, buff, port);
  //---------------------------------------------------------------------------------------------------------
  sock_fd= socket(AF_INET, SOCK_STREAM, 0);

	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}
	printf("TCP socket created. Ready to connect\n");
	server_addr.sin_family = AF_INET;
	// this values can be read from the keyboard
	server_addr.sin_port= htons(atoi(port));
	inet_aton(ip, &server_addr.sin_addr);

	if( -1 == connect(sock_fd,
			(const struct sockaddr *) &server_addr,
			sizeof(server_addr))){
				printf("Error connecting\n");
				exit(-1);
	}
  return sock_fd;
}

uint32_t gallery_add_photo(int peer_socket, char *file_n){

}

int gallery_add_keyword(int peer_socket, uint32_t id_photo, char *keyword){

}

int gallery_search_photo(int peer_socket, char * keyword, uint32_t ** id_photo){

}

int gallery_delete_photo(int peer_socket, uint32_t id_p){

}

int gallery_get_photo_name(int peer_socket, uint32_t id_photo, char **photo_name){

}

int gallery_get_photo(int peer_socket, uint32_t id_photo, char *file_name){

}
