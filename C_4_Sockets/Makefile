CC=gcc
CFLAGS=-O3 -DINFO

all: server_compile server_slave_compile client_compile control_server_compile
	@echo "Files were compiled!"

server_compile:
	@$(CC) $(CFLAGS) server/server.c Com_libs/log.c Com_libs/fast_use.c packet/packet.c ID/ID.c -o build/server.o
server_slave_compile:	
	@$(CC) $(CFLAGS) server/server_slave.c Com_libs/log.c Com_libs/fast_use.c packet/packet.c -o build/server_slave.o
client_compile:
	@$(CC) $(CFLAGS) client/client.c Com_libs/log.c Com_libs/fast_use.c packet/packet.c -o build/client.o
control_server_compile:
	@$(CC) $(CFLAGS) server/control_server.c Com_libs/log.c Com_libs/fast_use.c packet/packet.c -o build/control_server.o

close: control_server_compile
	@build/./control_server.o CLOSE_SERVER
server: server_compile
	@build/./server.o
client: client_compile
	@build/./client.o
client_broadcast: client_compile
	@build/./client.o BROADCAST

clean: 
	@rm build/control_server.o build/client.o build/server.o build/server_slave.o