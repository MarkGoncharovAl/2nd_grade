CC=gcc
CFLAGS=-O3 -DINFO -DMAX_INFO

LOG=build/COM_LIBS_LOG.o
FAST_USE=build/COM_LIBS_USE.o
FAST_USE_L=/usr/lib/libfast_use.so
PACK=build/PACKET.o
ID=build/ID.o
CRYPTO=build/crypto.o
CRYPTO_L=/usr/lib/libcrypto.so

COM_LIBS_O=$(LOG) $(PACK) $(ID) $(CRYPTO)
DYNAMIC=-lfast_use -lcrypto

all: server_compile server_slave_compile client_compile server_compileTCP server_slave_compileTCP client_compileTCP
	@echo "Files were compiled!"

server_slave_compile: COM_LIBS
	@$(CC) $(CFLAGS) -c server/server_slave.c -o build/server_slave.o
server_compile: COM_LIBS server_slave_compile
	@$(CC) $(CFLAGS) server/server.c $(COM_LIBS_O) build/server_slave.o -o build/server.o $(DYNAMIC)
client_compile: COM_LIBS
	@$(CC) $(CFLAGS) client/client.c $(COM_LIBS_O) -o build/client.o $(DYNAMIC)
server_compileTCP: COM_LIBS
	@$(CC) $(CFLAGS) server_TCP/server.c $(COM_LIBS_O) -o build/serverTCP.o $(DYNAMIC)
server_slave_compileTCP: COM_LIBS
	@$(CC) $(CFLAGS) server_TCP/server_slave.c $(COM_LIBS_O) -o build/server_slaveTCP.o $(DYNAMIC)
client_compileTCP: COM_LIBS
	@$(CC) $(CFLAGS) client/client_TCP.c $(COM_LIBS_O) -o build/clientTCP.o $(DYNAMIC)


server: server_compile
	@sudo build/./server.o
client: client_compile
	@sudo build/./client.o
client_broadcast: client_compile
	@sudo build/./client.o BROADCAST

serverTCP: server_compileTCP
	@sudo build/./serverTCP.o
clientTCP: client_compileTCP
	@sudo build/./clientTCP.o


COM_LIBS: COM_LIBS_LOG COM_LIBS_USE COM_LIBS_PACKET COM_LIBS_ID

COM_LIBS_LOG:
	@$(CC) $(CFLAGS) -c Com_libs/log.c -o $(LOG)
COM_LIBS_USE:
	@$(CC) $(CFLAGS) -c -fPIC Com_libs/fast_use.c -o $(FAST_USE)
	@$(CC) $(CFLAGS) -shared $(FAST_USE) -o build/libfast_use.so
	@sudo cp build/libfast_use.so /usr/lib
	@sudo chmod 0755 $(FAST_USE_L)

COM_LIBS_CRYPTO: COM_LIBS_LOG
	@$(CC) $(CFLAGS) -c -fPIC crypto/crypto.c -o $(CRYPTO)
	@$(CC) $(CFLAGS) -shared $(CRYPTO) -o build/libcrypto.so
	@sudo cp build/libcrypto.so /usr/lib
	@sudo chmod 0755 $(CRYPTO_L)

COM_LIBS_PACKET: COM_LIBS_LOG COM_LIBS_CRYPTO
	@$(CC) $(CFLAGS) -c packet/packet.c -o $(PACK)
COM_LIBS_ID: COM_LIBS_LOG COM_LIBS_PACKET
	@$(CC) $(CFLAGS) -c ID/ID.c -o $(ID)

clean: 
	@rm build/*
	@sudo rm $(FAST_USE_L)
	@sudo rm $(CRYPTO_L)