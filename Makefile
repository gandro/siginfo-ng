#
# Makefile for siginfo-ng
#

OBJECTS=siginfo-ng.o lua_helper.o lua_plugin.o lua_settings.o siginfo.o
CFLAGS=-Wall -O
LDFLAGS=-lm -llua

#
# siginfo-ng settings
#

# Platform architecture (stolen from Linux makefile)
OS=$(shell uname -s)
ARCH=$(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ \
			-e s/arm.*/arm/ -e s/sa110/arm/ \
			-e s/s390x/s390/ -e s/parisc64/parisc/ \
			-e s/ppc.*/powerpc/ -e s/mips.*/mips/ \
			-e s/sh[234].*/sh/ )

# Remove this if your libc does not support getopt_long
OPTIONS=-DUSE_GETOPT_LONG

#
# Lua settings
#

LUA=lua-5.1.4
LUA_PLATFORM=posix
LUA_PREFIX=$(LUA)
LUA_LDFLAGS=-lm

all: siginfo-ng plugins

siginfo-ng: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

%.o: %.c siginfo-ng.h
	$(CC) $(CFLAGS) $(OPTIONS) -c $<

include-lua: $(LUA_PREFIX)
	$(MAKE) all CFLAGS="$(CFLAGS) -I$(LUA_PREFIX)/include/" LDFLAGS="$(LUA_LDFLAGS)" \
		OBJECTS="$(OBJECTS) $(LUA_PREFIX)/lib/liblua.a"

plugins:
	mkdir plugins/
	cp -v platform/*.lua plugins/
	cp -v platform/$(OS)/*.lua plugins/
	cp -v platform/$(OS)/$(ARCH)/*.lua plugins/

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
	rm -rf plugins/
	rm -f $(OBJECTS)
	rm -f "$(LUA).tar.gz"
	rm -rf $(LUA)

echo:
	@echo "OS      : $(OS)"
	@echo "ARCH    : $(ARCH)"
	@echo "LUA     : $(LUA)"
	@echo "OPTIONS : $(OPTIONS)"

.PHONY: include-lua install uninstall forced-uninstall clean
