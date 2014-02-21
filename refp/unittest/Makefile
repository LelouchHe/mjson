CC = gcc
AR = ar

SRC 	= $(wildcard *.c)
OBJS 	= $(SRC:%.c=%.o)
EXE		= test

INCLUDE	= -I..

LDFLAGS	= -L.. -lrefp

CFLAGS	= -g -Wall -DMJSON_DEBUG

all: $(EXE)


$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.c.o:
	$(CC) -c $^ $(CFLAGS) $(INCLUDE)

clean:
	rm -f $(EXE) $(OBJS)
