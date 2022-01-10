# Grace Biemans (geb965) & Nick Heleta

.PHONY: clean

Groomers: petgroomsynch.o
	gcc -o Groomers petgroomsynch.o

petgroomsynch.o: petgroomsynch.c petgroomsynch.h
	gcc -Wall -Wextra -c petgroomsynch.c

clean:
	rm -f *.o Groomers
