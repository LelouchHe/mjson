CC = gcc
AR = ar

SRC 	= mjson_core_test.c ../mjson_core.c
OBJS 	= $(SRC:%.c=%.o)
EXE		= mjson_core_test

INCLUDE	= -I..

LDFLAGS	= -L../lib -lrefp -lutil

CFLAGS	= -g -Wall -DMJSON_DEBUG

all: $(EXE)


$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

.c.o:
	$(CC) -c $^ $(CFLAGS) $(INCLUDE)

clean:
	rm -f $(EXE) $(OBJS)
