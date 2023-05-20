CC=gcc
FLAGS=-Wall#-g -ggdb3
RPC_SYSTEM=rpc.o
RPC_SERVER=rpc-server
RPC_CLIENT=rpc-client

.PHONY: format all

all: $(RPC_SYSTEM) $(RPC_SERVER) $(RPC_CLIENT)

$(RPC_SYSTEM): rpc.c rpc.h
	$(CC) -c $(FLAGS) -o $@ $<

server.o: server.c rpc.h
	$(CC) -c $(FLAGS) -o $@ $<

client.o: client.c rpc.h
	$(CC) -c $(FLAGS) -o $@ $<

$(RPC_SERVER): server.o $(RPC_SYSTEM)
	$(CC) -o $(RPC_SERVER) $(FLAGS) server.o $(RPC_SYSTEM)

$(RPC_CLIENT): client.o $(RPC_SYSTEM)
	$(CC) -o $(RPC_CLIENT) $(FLAGS) client.o $(RPC_SYSTEM)


format:
	clang-format -style=file -i *.c *.h


clean:
	rm -f *.o $(RPC_SERVER) $(RPC_CLIENT)
