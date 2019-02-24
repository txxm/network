CC = gcc
CFLAGS = -g -Wall -lcrypto -lssl -L/usr/local/lib -lzlog -lpthread
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
	$(COPY) $(IMAGE) netdisk_client
	$(COPY) netdisk_client /usr/bin
	rm -f netdisk_client

.PHONY:clean
clean:
	$(ECHO) rm -f main
