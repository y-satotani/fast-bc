MAKE := make
.PHONY: all, install, clean, uninstall

all:
	$(MAKE) -C dybc all
	$(MAKE) -C exp all
	$(MAKE) -C test all

debug:
	$(MAKE) -C dybc debug
	$(MAKE) -C exp debug
	$(MAKE) -C test debug

install:
	mkdir -p bin
	cp exp/*.out test/*.out bin

clean:
	$(MAKE) -C dybc clean
	$(MAKE) -C exp clean
	$(MAKE) -C test clean

uninstall:
	rm -rf bin
