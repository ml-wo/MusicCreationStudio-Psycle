CC = gcc
inc = -I../../container/src -I../../../diversalis/src
src = $(wildcard *.c)
obj = $(src:.c=.o)

libfile.a: $(obj)
	ar rsv $@ $^

%.o: %.c
	$(CC) $(inc) -c $(src)

.PHONY: clean	
clean:
	rm -f $(obj) libfile.a
