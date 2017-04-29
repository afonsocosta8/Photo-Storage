all:
	gcc -std=c99 gateway.c -o gateway 
	gcc -std=c99 client.c photostorageapi.c -o client
debug:
	gcc -std=c99 -DDEBUG client.c photostorageapi.c -o client
	gcc -std=c99 -DDEBUG gateway.c -o gateway
gateway:
	gcc -std=c99 gateway.c -o gateway
gatewaydebug:
	gcc -std=c99 -DDEBUG gateway.c -o gateway
clientdebug:
	gcc -DDEBUG -g -std=c99 client.c photostorageapi.c -o client
client: client.c photostorageapi.c
	gcc -o client -g client.c photostorageapi.c
