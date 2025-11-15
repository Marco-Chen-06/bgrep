CC = gcc
CFLAGS += -g

bgrep : bgrep.c
	${CC} ${CFLAGS} -o $@ $^

.PHONY: clean

clean:
	@rm -rvf *.o