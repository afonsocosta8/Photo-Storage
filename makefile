all:
	gcc -Wall -std=c99 -pthread data_structs.c gateway.c -o gateway
	gcc -Wall -std=c99 client.c photostorageapi.c -o client
	gcc -Wall -std=c99 -pthread peer.c -o peer
debug:
	gcc -Wall -std=c99 -DDEBUG client.c photostorageapi.c -o client
	gcc -Wall -std=c99 -pthread -DDEBUG data_structs.c gateway.c -o gateway
	gcc -Wall -std=c99 -pthread -DDEBUG peer.c -o peer
afonso:
	gcc -Wall -DDEBUG client.c photostorageapi.c -o client
	gcc -Wall -pthread -DDEBUG data_structs.c gateway.c -o gateway
	gcc -Wall -pthread -DDEBUG peer.c -o peer

gateway:
	gcc -Wall -std=c99 -pthread data_structs.c gateway.c -o gateway
gatewaydebug:
	gcc -Wall -std=c99 -pthread -DDEBUG data_structs.c gateway.c -o gateway
clientdebug:
	gcc -Wall -DDEBUG -g -std=c99 client.c photostorageapi.c -o client
client:
	gcc -Wall -o client -g client.c photostorageapi.c
peer:
	gcc -Wall -std=c99 -pthread peer.c -o peer
peerdebug:
	gcc -Wall -std=c99 -pthread -DDEBUG peer.c -o peer
datastruct:
	gcc -Wall -std=c99 data_structs.c -o datastructs
