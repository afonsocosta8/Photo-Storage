typedef struct _peer{

  char ip[16];
  int  port;
  struct _peer *next;

}peer;


typedef struct _peerlist{

  peer *next_to_use;
  peer *beginning;

}peer_list;

typedef struct _brotherlist{

  peer *last;
  peer *first;

}brother_list;



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
  pthread_mutex_t lock;

}photo_list;


typedef struct _photohashtable{

  photo_list **table;
  int total;
  int size;


}photo_hash_table;


// PEER LISTS FOR GATEWAY

peer_list *init_peer_list();

void add_peer_list(peer_list *list, char *ip, int port);

void print_peer_list(peer_list *list);

int get_peer(peer_list *list, char *ip, int *port);

void remove_peer(peer_list *list, char *ip, int port);

void free_peer_list(peer_list *list);

peer_list *init_peer_list();


// PEER LISTS FOR PEERS

brother_list *init_brother_list();

void add_brother_list(brother_list *list, char *ip, int port);

void print_brother_list(brother_list *list);

int get_brother(brother_list *list, char *ip, int *port);

void remove_brother(brother_list *list, char *ip, int port);

void free_brother_list(brother_list *list);

brother_list *init_brother_list();


// PHOTO LISTS

photo_hash_table * create_hash_table(int size);

void free_hash_table(photo_hash_table *table);

void add_photo_hash_table(photo_hash_table *table, char *name, uint32_t id);

int delete_photo_hash(photo_hash_table *table, uint32_t id);

int add_keyword_photo_hash(photo_hash_table *table, uint32_t id, char *keyword);

int get_photo_name_hash(photo_hash_table *table, uint32_t id, char *name);

void print_photo_hash(photo_hash_table *table);
