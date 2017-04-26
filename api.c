#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

int gallery_connect(char * host, in_port_t p){

  struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	char buff[]="GET";
	int nbytes;

	int sock_fd= socket(AF_INET, SOCK_DGRAM, 0);

	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

	printf(" socket created \n Ready to send\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(p);
	inet_aton(host, &server_addr.sin_addr);

	sprintf(buff, "GET");
	nbytes = sendto(sock_fd,
	                    buff, strlen(buff)+1, 0,
	                    (const struct sockaddr *) &server_addr, sizeof(server_addr));
	printf("sent %d %s\n", nbytes, buff);
	nbytes = recv(sock_fd, buff, 20, 0);
	printf("received %d bytes --- %s ---\n", nbytes, buff);
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
