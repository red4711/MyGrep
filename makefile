OBJECTS=search.o
CC=gcc
CFLAGS=-std=c99 -Wall -ggdb -lpthread

search: $(OBJECTS)
	$(CC) $(CFLAGS) $? -o $@

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf $(OBJECTS)
