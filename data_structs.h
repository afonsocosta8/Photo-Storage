typedef struct _peer{

  char ip[16];
  int  port;
  struct _peer *next;

}peer;


typedef struct _peerlist{

  int total;
  peer *next_to_use;
  peer *beginning;
  pthread_mutex_t lock;

}peer_list;

typedef struct _brotherlist{

  int total;
  peer *last;
  peer *first;
  pthread_mutex_t lock;

}brother_list;



typedef struct _key_word{

  char key[100];
  struct _key_word *next;

}key_word;


typedef struct _keywordlist{

  key_word *list;
  int total;

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

char **get_all_peers(peer_list *list, int* total);

int remove_peer(peer_list *list, char *ip, int port);

void free_peer_list(peer_list *list);


// PEER LISTS FOR PEERS

brother_list *init_brother_list();

void add_brother_list(brother_list *list, char *ip, int port);

void print_brother_list(brother_list *list);

char **get_all_brothers(brother_list * list, int *total);

int remove_brother(brother_list *list, char *ip, int port);

void free_brother_list(brother_list *list);

int find_peer(peer_list *list, char *ip, int port);

// PHOTO LISTS

keyword_list *init_keyword_list();

photo_hash_table * create_hash_table(int size);

void free_hash_table(photo_hash_table *table);

void free_keyword_list(keyword_list *list);

void add_photo_hash_table(photo_hash_table *table, char *name, uint32_t id);

int delete_photo_hash(photo_hash_table *table, uint32_t id);

void add_keyword_list(keyword_list *keys, char *key);

int add_keyword_photo_hash(photo_hash_table *table, uint32_t id, char *keyword);

int get_photo_name_hash(photo_hash_table *table, uint32_t id, char *name);

void print_keyword_list(keyword_list *list);

void print_photo_hash(photo_hash_table *table);
