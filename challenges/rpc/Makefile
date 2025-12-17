CC = gcc
CFLAGS = -I. -Wall
DEPS = base64.c compression.c encryption.c network.c sort.c
OBJ_RPC = rpc.o $(DEPS:.c=.o)
OBJ_SOLVE = solve.o $(DEPS:.c=.o)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rpc: $(OBJ_RPC)
	$(CC) -o $@.out $^ $(CFLAGS)
	strip -s $@.out

solve: $(OBJ_SOLVE)
	$(CC) -o $@.out $^ $(CFLAGS)

.PHONY: clean

clean:
	$(RM) $(OBJ_RPC) rpc.out $(OBJ_SOLVE) solve.out
