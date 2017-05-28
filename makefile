all:
	gcc -Wall -pthread data_structs.c gateway.c -o gateway
	gcc -Wall client.c photostorageapi.c data_structs.c -o client
	gcc -Wall -pthread peer.c data_structs.c -o peer
	gcc -Wall client2.c photostorageapi.c data_structs.c -o client2
debug:
	gcc -Wall -g -DDEBUG  photostorageapi.c data_structs.c client.c -o client
	gcc -Wall -g -DDEBUG  photostorageapi.c data_structs.c client2.c -o client2
	gcc -Wall -g -pthread -DDEBUG data_structs.c gateway.c -o gateway
	gcc -Wall -g -pthread -DDEBUG data_structs.c peer.c -o peer
afonso:
	gcc -Wall -DDEBUG client.c photostorageapi.c data_structs.c -o client
	gcc -Wall -pthread -DDEBUG data_structs.c gateway.c -o gateway
	gcc -Wall -pthread -DDEBUG peer.c data_structs.c -o peer

gateway:
	gcc -Wall -pthread data_structs.c gateway.c -o gateway
gatewaydebug:
	gcc -Wall -pthread -DDEBUG data_structs.c gateway.c -o gateway
clientdebug:
	gcc -Wall -DDEBUG -g client.c photostorageapi.c -o client
client:
	gcc -Wall -o client -g client.c photostorageapi.c
peer:
	gcc -Wall -pthread peer.c data_structs.c -o peer
peerdebug:
	gcc -Wall -pthread -DDEBUG data_structs.c peer.c  -o peer
datastruct:
	gcc -Wall data_structs.c -o datastructs
