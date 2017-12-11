CC      := g++
CFLAGS  := -Wall -Wextra -I./lame-3.100/include
DEFINES := -DDEBUG
APPNAME := lametest

$(APPNAME): main.o lame-3.100/libmp3lame/.libs/libmp3lame.a
	$(CC) -o $@ -pthread $^

lame-3.100/libmp3lame/.libs/libmp3lame.a:
	cd lame-3.100;./configure
	make -C lame-3.100

main.o: main.cpp platform.h thread_signal.h wavreader.h lame_enc.h Makefile
	$(CC) -c $(CFLAGS) $(DEFINES) -o $@ main.cpp

.PHONY: clean distclean

clean:
	rm -rf $(APPNAME) *.o
	make -C lame-3.100 distclean

distclean: clean
	rm -rf *~ log*
