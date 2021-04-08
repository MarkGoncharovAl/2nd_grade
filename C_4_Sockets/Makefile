CC=gcc
CFLAGS=-O3 -DINFO -DMAX_INFO

LOG=build/COM_LIBS_LOG.o
FAST_USE=build/COM_LIBS_USE.o
PACK=build/PACKET.o
ID=build/ID.o

COM_LIBS_O=$(LOG) $(FAST_USE) $(PACK) $(ID)

all: server_compile server_slave_compile client_compile control_server_compile server_compileTCP server_slave_compileTCP client_compileTCP
	@echo "Files were compiled!"

server_compile: COM_LIBS
	@$(CC) $(CFLAGS) server/server.c $(COM_LIBS_O) -o build/server.o
server_slave_compile: COM_LIBS
	@$(CC) $(CFLAGS) server/server_slave.c $(COM_LIBS_O) -o build/server_slave.o
client_compile: COM_LIBS
	@$(CC) $(CFLAGS) client/client.c $(COM_LIBS_O) -o build/client.o
control_server_compile: COM_LIBS
	@$(CC) $(CFLAGS) server/control_server.c $(COM_LIBS_O) -o build/control_server.o
server_compileTCP: COM_LIBS
	@$(CC) $(CFLAGS) server_TCP/server.c $(COM_LIBS_O) -o build/serverTCP.o
server_slave_compileTCP: COM_LIBS
	@$(CC) $(CFLAGS) server_TCP/server_slave.c $(COM_LIBS_O) -o build/server_slaveTCP.o
client_compileTCP: COM_LIBS
	@$(CC) $(CFLAGS) client/client_TCP.c $(COM_LIBS_O) -o build/clientTCP.o


close: control_server_compile
	@build/./control_server.o CLOSE_SERVER
server: server_compile
	@build/./server.o
client: client_compile
	@build/./client.o
client_broadcast: client_compile
	@build/./client.o BROADCAST

serverTCP: server_compileTCP
	@build/./serverTCP.o
clientTCP: client_compileTCP
	@build/./clientTCP.o


COM_LIBS: COM_LIBS_LOG COM_LIBS_USE COM_LIBS_PACKET COM_LIBS_ID

COM_LIBS_LOG: 
	@$(CC) $(CFLAGS) -c Com_libs/log.c -o $(LOG)
COM_LIBS_USE:
	@$(CC) $(CFLAGS) -c Com_libs/fast_use.c -o $(FAST_USE)
COM_LIBS_PACKET: COM_LIBS_LOG
	@$(CC) $(CFLAGS) -c packet/packet.c -o $(PACK)
COM_LIBS_ID: COM_LIBS_LOG COM_LIBS_PACKET
	@$(CC) $(CFLAGS) -c ID/ID.c -o $(ID)

clean: 
	@rm build/*.o