#
# Makefile for siginfo-ng
#

OBJECTS=siginfo-ng.o lua_helper.o lua_plugin.o lua_settings.o siginfo.o
CFLAGS=-Wall -O
LDFLAGS=-lm -llua

#
# siginfo-ng settings
#

# Remove this if your libc does not support getopt_long
OPTIONS=-DUSE_GETOPT_LONG

#
# Lua settings
#

LUA=lua-5.1.4
LUA_PLATFORM=posix
LUA_PREFIX=$(LUA)
LUA_LDFLAGS=-lm

all: siginfo-ng

siginfo-ng: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.c siginfo-ng.h
	$(CC) $(CFLAGS) $(OPTIONS) -c $<

include-lua: $(LUA_PREFIX)
	$(MAKE) all CFLAGS="$(CFLAGS) -I$(LUA_PREFIX)/include/" LDFLAGS="$(LUA_LDFLAGS)" \
		OBJECTS="$(OBJECTS) $(LUA_PREFIX)/lib/liblua.a"

$(LUA):
	wget "http://www.lua.org/ftp/$(LUA).tar.gz" -O "$(LUA).tar.gz"
	tar xzf "$(LUA).tar.gz"
	cd $(LUA) && make $(LUA_PLATFORM) local

install: siginfo-ng
	install -D -m 755 siginfo-ng $(DESTDIR)/usr/bin/siginfo-ng
	install -D -b -m 644 config.lua $(DESTDIR)/etc/siginfo-ng/config.lua
	install -d -m 755 $(DESTDIR)/etc/siginfo-ng/plugins
	install -m 644 plugins/* $(DESTDIR)/etc/siginfo-ng/plugins

forced-uninstall:
	$(MAKE) uninstall FORCE=-f

uninstall:
	rm -f $(DESTDIR)/usr/bin/siginfo-ng
	rm -i $(FORCE) -r $(DESTDIR)/etc/siginfo-ng/

clean:
	rm -f siginfo-ng
	rm -f $(OBJECTS)
	rm -f "$(LUA).tar.gz"
	rm -rf $(LUA)

.PHONY: include-lua install clean
