CC = gcc
AR = ar

SRC 	= $(wildcard *.c)
OBJS 	= $(SRC:%.c=%.o)
LIB 	= libmjson.a

INCLUDE	= 

LDFLAGS	=

CFLAGS	= -g -Wall -DMJSON_DEBUG

all: $(LIB)
	cp $(LIB) lib/

$(LIB): $(OBJS)
	$(AR) -r $@ $^

.c.o:
	$(CC) -c $^ $(CFLAGS) $(INCLUDE)

clean:
	rm -f $(LIB) $(OBJS)
