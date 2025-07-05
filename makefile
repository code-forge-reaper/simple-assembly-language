all: libsal.a sal test.so

sal: src/sal.c
	cc src/main.c src/sal.c -o sal -Wextra -Werror -ggdb -l raylib -lm
libsal.a:src/sal.c
	cc -fPIC -c src/sal.c -o sal.o
	ar rcs libsal.a sal.o

test.so: src/test.c libsal.a
	cc -fPIC -shared -o test.so src/test.c -L. -lsal
clean:
	rm -f libsal.a sal sal.o test.so
