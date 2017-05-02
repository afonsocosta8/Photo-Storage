typedef struct _peer{

  char ip[16];
  int  port;
  struct _peer *next;

}peer;


typedef struct _peerlist{

  peer *next_to_use;
  peer *beginning;

}peer_list;


typedef struct _keyword{

  char key[100];
  struct _keyword *next;

}keyword;


typedef struct _keywordlist{

  keyword *list;

}keyword_list;


typedef struct _photo{

  uint32_t id;
  char name[100];
  keyword_list *keywords;
  struct _photo *next;

}photo;

typedef struct _photolist{

  photo *list;
  int total;

}photo_list;



peer_list *init_peer_list();

void add_peer_list(peer_list *list, char *ip, int port);

void print_peer_list(peer_list *list);

int get_peer(peer_list *list, char *ip, int *port);

void remove_peer(peer_list *list, char *ip, int port);

void free_peer_list(peer_list *list);
