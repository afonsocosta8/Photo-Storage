int gallery_connect(char * host, in_port_t p){

  struct sockaddr_un server_addr;
	char buff[100];
	int nbytes;

	int sock_fd= socket(AF_UNIX, SOCK_DGRAM, 0);

	if (sock_fd == -1){
		perror("socket: ");
		exit(-1);
	}

  printf(" socket created \n Ready to send\n");

	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, SOCK_ADDRESS);

	sprintf(buff, "GET");
	nbytes = sendto(sock_fd,
	                    buff, strlen(buff)+1, 0,
	                    (const struct sockaddr *) &server_addr, sizeof(server_addr));
	printf("sent %d %s\n", nbytes, buff);
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
