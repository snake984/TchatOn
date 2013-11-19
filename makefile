CC=gcc
CFLAGS += -Wall -g
LDFLAGS += -pthread

TARGET = tchat

all : $(TARGET)

clean :
	rm $(TARGET)
