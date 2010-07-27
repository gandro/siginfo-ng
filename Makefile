CC=gcc
CFLAGS=-Wall -O
LDFLAGS=-llua -ldl -lm

OBJ=siginfo-ng.o lua_helper.o lua_plugin.o lua_settings.o siginfo.o

all: siginfo-ng

siginfo-ng: $(OBJ)
	$(CC) $(CFLAGS) $+ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

install: siginfo-ng
	$(error "'make install' is not yet implemented!")


clean:
	rm -f siginfo-ng $(OBJ)
