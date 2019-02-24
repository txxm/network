CC = gcc
CFLAGS = -g -Wall -lcrypto -lssl -lzlog -lpthread
INCLUDE = -I ../include/
ECHO = @
COPY = cp

IMAGE = main
SRC = $(wildcard *.c)
OBJ = $(patsubst *.c, *.o, $(SRC))

$(IMAGE):$(OBJ)
	$(ECHO) $(CC) $(CFLAGS) $(INCLUDE) $(OBJ) -o $(IMAGE) 

%.o:%.c
	$(ECHO) $(CC) $(CFLAGS) $(INCLUDE) -c $<

.PHONY:install
install:
	$(COPY) $(IMAGE) netdisk_server
	$(COPY) netdisk_server /usr/bin/
	rm -f netdisk_server

.PHONY:clean
clean:
	$(ECHO) rm -f *.o
	$(ECHO) rm -f main
