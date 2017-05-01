typedef struct _peer{

  char ip[16];
  int  port;
  struct _peer *next;

}peer;


typedef struct _peerlist{

  peer *next_to_use;
  peer *beginning;

}peer_list;

typedef struct _args{

  struct sockaddr_in client_addr;
  peer_list * list;

}args;


peer_list *init_peer_list();
void add_peer_list(peer_list *list, char *ip, int port);
void print_peer_list(peer_list *list);
int get_peer(peer_list *list, char *ip, int *port);
void remove_peer(peer_list *list, char *ip, int port);
void free_peer_list(peer_list *list);
