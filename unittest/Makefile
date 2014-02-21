CC = gcc
AR = ar

SRC 	= mjson_core_test.c
OBJS 	= $(SRC:%.c=%.o)
EXE		= test

INCLUDE	= -I..

LDFLAGS	= -L../lib -lmjson -lutil -lrefp

CFLAGS	= -g -Wall -DMJSON_DEBUG

all: $(EXE)


$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.c.o:
	$(CC) -c $^ $(CFLAGS) $(INCLUDE)

clean:
	rm -f $(EXE) $(OBJS)
