CC = gcc
inc = -I../../container/src -I../../../diversalis/src -I /usr/include/freetype2
cflags = -Wfatal-errors -g
src = $(wildcard *.c)
obj = $(src:.c=.o)

libpsyui.a: $(obj)
	ar rsv $@ $^

%.o: %.c
	$(CC) $(cflags) $(inc) -c $(src);

.PHONY: clean	
clean:
	rm -f $(obj) libpsyui.a
