CC = clang
ARGS = -Wall

all: read_usb server

read_usb: 
	$(CC) -o $(ARGS) read_usb.c

server: read_usb
	$(CC) -o $(ARGS) -g -lpthread server.c

clean: 
	rm -rf read_usb server

