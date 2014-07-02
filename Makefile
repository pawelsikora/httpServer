CC=gcc
CFLAGS=-I. -g `pkg-config --cflags glib-2.0` `pkg-config --libs glib-2.0`
OBJ = server.o

server: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
	
clean:
	rm -rf server
