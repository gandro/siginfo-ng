-include .plugins

all: siginfo-ng

siginfo-ng: siginfo-ng.o http.o plugin.o $(PLUGINS_O) init_plugins.o
	$(CC) $(CFLAGS) $+ $(LDFLAGS) -o $@

config:
	@rm -f .plugins
	@for plugin in plugins/*.c; do \
		plugin_name=`basename $${plugin} .c`; \
		echo -n "Use plugin '$${plugin_name}'? [Y/n] "; \
		read use; \
		if [ -z "$${use}" ]||[ "$${use}" = "y" ]||[ "$${use}" = "Y" ]; then \
			echo "PLUGINS_C+=$${plugin}">>.plugins; \
		else \
			echo "#PLUGINS_C+=$${plugin}">>.plugins; \
		fi; \
	done;
	@echo 'PLUGINS_O=$$(addsuffix .o, $$(basename $$(PLUGINS_C)))'>>.plugins
	@echo 'PLUGINS=$$(notdir $$(basename $$(PLUGINS_C)))'>>.plugins

clean:
	rm -f siginfo-ng
	rm -f init_plugins.c
	rm -f plugins/*.o
	rm -f *.o

install: siginfo-ng
	install -m 755 siginfo-ng $(DESTDIR)/usr/bin
	install -m 644 docs/sample.conf $(DESTDIR)/etc/siginfo-ng.conf

%.o: %.c
	$(CC) $(CFLAGS) -c $< $(LDFLAGS)

$(PLUGINS_O): $(PLUGINS_C)
ifndef PLUGINS
	$(error No plugin list found. Please run "make config" first!)
endif
	$(foreach PLUGIN, $(PLUGINS_C), \
		$(CC) $(CFLAGS) -c $(PLUGIN) $(LDFLAGS) \
			-o $(addsuffix .o, $(basename $(PLUGIN)));)

init_plugins.c:
ifndef PLUGINS
	$(error No plugin list found. Please run "make config" first!)
endif
	echo "/* this file was auto-generated by 'make' */">init_plugins.c
	$(foreach PLUGIN, $(PLUGINS), \
		echo "extern void $(PLUGIN)_init();">>init_plugins.c;)
	echo >>init_plugins.c
	echo "void init_plugins() {">>init_plugins.c
	$(foreach PLUGIN, $(PLUGINS), \
		echo "    $(PLUGIN)_init();">>init_plugins.c;)
	echo "}">>init_plugins.c

