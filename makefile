MODS:= test.so rl.so
all: libsal.a sal ${MODS}

sal: interp/sal.c
	cc src/main.c interp/sal.c -o sal -Wextra -Werror -ggdb

libsal.a:interp/sal.c
	cc -fPIC -c interp/sal.c -o sal.o
	ar rcs libsal.a sal.o

test.so: mods/test.c libsal.a
	cc -fPIC -shared -o test.so mods/test.c -L. -lsal

rl.so: mods/rl.c libsal.a
	cc -fPIC -shared -o rl.so mods/rl.c -L. -lsal  -l raylib -lm -l X11 -fPIC

clean:
	rm -f libsal.a sal sal.o ${MODS}
