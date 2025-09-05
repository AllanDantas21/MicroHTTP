NAME = httpc

.PHONY: all lib clean fclean re install help

all: lib

lib:
	$(MAKE) -f Makefile.lib all

clean:
	$(MAKE) -f Makefile.lib clean

fclean:
	$(MAKE) -f Makefile.lib clean-all

re: fclean all

install:
	$(MAKE) -f Makefile.lib install

help:
	$(MAKE) -f Makefile.lib help
