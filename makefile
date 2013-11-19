CC=gcc
CFLAGS += -Wall -g
LDFLAGS += -pthread

TARGET = tchat r_client w_client

all : $(TARGET)

clean :
	rm $(TARGET)
