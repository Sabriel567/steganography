CFLAGS = -O0

steno.out: main.o image.o steno.o
	gcc $(CFLAGS) steno.o image.o main.o  -o steno.out

main.o: main.c image.h steno.h
	gcc $(CFLAGS) main.c -c

steno.o: steno.c
	gcc $(CFLAGS) steno.c -c

image.o: image.c
	gcc $(CFLAGS) image.c -c

clean:
	rm -f *.o
