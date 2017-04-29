all: 
	gcc -std=c99 gateway.c -o gateway 
	gcc -std=c99 client.c photostorageapi.c -o client
	gcc -std=c99 peer.c -o peer
debug:
	gcc -std=c99 -DDEBUG client.c photostorageapi.c -o client
	gcc -std=c99 -DDEBUG gateway.c -o gateway
	gcc -std=c99 -DDEBUG peer.c -o peer
gateway:
	gcc -std=c99 gateway.c -o gateway
gatewaydebug:
	gcc -std=c99 -DDEBUG gateway.c -o gateway
clientdebug:
	gcc -DDEBUG -g -std=c99 client.c photostorageapi.c -o client
client:
	gcc -o client -g client.c photostorageapi.c
peer:
	gcc -std=c99 peer.c -o peer
peerdebug:
	gcc -std=c99 -DDEBUG peer.c -o peer

