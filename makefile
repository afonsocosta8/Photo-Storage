all: 
	gcc -std=c99 -pthread gateway.c -o gateway 
	gcc -std=c99 client.c photostorageapi.c -o client
	gcc -std=c99 -pthread peer.c -o peer
debug:
	gcc -DDEBUG client.c photostorageapi.c -o client
	gcc -pthread -DDEBUG gateway.c -o gateway
	gcc -pthread -DDEBUG peer.c -o peer
gateway:
	gcc -std=c99 -pthread gateway.c -o gateway
gatewaydebug:
	gcc -std=c99 -pthread -DDEBUG gateway.c -o gateway
clientdebug:
	gcc -DDEBUG -g -std=c99 client.c photostorageapi.c -o client
client:
	gcc -o client -g client.c photostorageapi.c
peer:
	gcc -std=c99 -pthread peer.c -o peer
peerdebug:
	gcc -std=c99 -pthread -DDEBUG peer.c -o peer

