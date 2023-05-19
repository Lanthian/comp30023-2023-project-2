CC=gcc
RPC_SYSTEM=rpc.o
RPC_SERVER=rpc-server
RPC_CLIENT=rpc-client

.PHONY: format all

all: $(RPC_SYSTEM) $(RPC_SERVER) $(RPC_CLIENT)

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) -c -o $@ $<

server.o: server.c rpc.h
	$(CC) -c -o $@ $<

client.o: client.c rpc.h
	$(CC) -c -o $@ $<

$(RPC_SERVER): server.o $(RPC_SYSTEM)
	$(CC) -o $(RPC_SERVER) server.o $(RPC_SYSTEM)

$(RPC_CLIENT): client.o $(RPC_SYSTEM)
	$(CC) -o $(RPC_CLIENT) client.o $(RPC_SYSTEM)

# RPC_SYSTEM_A=rpc.a
# $(RPC_SYSTEM_A): rpc.o
# 	ar rcs $(RPC_SYSTEM_A) $(RPC_SYSTEM)

format:
	clang-format -style=file -i *.c *.h


clean:
	rm -f *.o $(RPC_SERVER) $(RPC_CLIENT)
	rm -f server client
