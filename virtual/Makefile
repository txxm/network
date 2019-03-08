CC = gcc
CFLAGS = -g -Wall

SRC = $(wildcard *.c)
OBJ = $(patsubst %.c, %.o, $(SRC))
TARGET = iftap

$(TARGET):$(OBJ)
	$(CC) $(CFLAGS) -o $@ $^
$(OBJ):$(SRC)
	$(CC) $(CFLAGS) -c $<

install:
	cp -f $(TARGET) /bin/

.PHONY:clean
clean:
	rm -rf *.o
	rm -rf $(TARGET)
