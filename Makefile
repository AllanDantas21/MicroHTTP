NAME = httpc

.PHONY: all lib clean fclean re install test help

all: lib

lib:
	$(MAKE) -f Makefile.lib all

test: lib
	$(MAKE) -C tests test

test-valgrind: lib
	$(MAKE) -C tests test-valgrind

test-debug: lib
	$(MAKE) -C tests test-debug

clean:
	$(MAKE) -f Makefile.lib clean
	$(MAKE) -C tests clean

fclean:
	$(MAKE) -f Makefile.lib clean-all
	$(MAKE) -C tests clean

re: fclean all

install:
	$(MAKE) -f Makefile.lib install
