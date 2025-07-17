hexcrc: hexcrc.c
	$(CC) -Wall -Wextra -std=c99 -pedantic -g -o $@ $<

clean:
	$(RM) hexcrc
