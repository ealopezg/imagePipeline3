CC = gcc

all: clean pipeline

pipeline: obj/image.o obj/utils.o 
	$(CC) src/main.c obj/image.o obj/utils.o -o pipeline -lm -Wall -pthread

obj/utils.o:
	$(CC) -c -Iinclude src/utils.c -o obj/utils.o -Wall

obj/image.o:
	$(CC) -c -Iinclude src/image.c -o obj/image.o -Wall

clean:
	rm -f obj/*.o
	rm -f pipeline
	rm -f pipe