CC=gcc
CFLAGS=-g -Wall
TARGET=aio

$(TARGET):aio.o
	$(CC) -o aio $^ -lrt

%.o:%.c
	$(CC) -c $< 

.PHONY:clean
clean:
	rm -rf aio.o aio

