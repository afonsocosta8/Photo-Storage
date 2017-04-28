gateway:
	gcc -Wall -std=c99 gateway.c -o gateway
gatewaydebug:
	gcc -Wall -std=c99 -DDEBUG gateway.c -o gateway

